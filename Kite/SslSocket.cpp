#include "SslSocket.hpp"
#include <stdio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <iostream>

using namespace Kite;

//TODO: not thread safe
static std::map<SSL  *, SslSocket *> rebind_map;

struct Kite::SslSocketPrivate {
    SSL_CTX *ssl_ctx;
    BIO     *bio;
    SSL     *ssl; //do note delete, is a ref from bio
    SslSocket::SocketState state;
    std::string errorMessage;
};

void apps_ssl_info_callback(const SSL *s, int where, int ret)
{
    const char *str;
    int w;
    w=where& ~SSL_ST_MASK;
    if (w & SSL_ST_CONNECT) str="SSL_connect";
    else if (w & SSL_ST_ACCEPT) str="SSL_accept";
    else str="undefined";
    if (where & SSL_CB_LOOP)
    {
        fprintf(stderr,"%s:%s\n",str,SSL_state_string_long(s));
    }
    else if (where & SSL_CB_ALERT)
    {
        str=(where & SSL_CB_READ)?"read":"write";
        fprintf(stderr,"SSL3 alert %s:%s:%s\n",
                str,
                SSL_alert_type_string_long(ret),
                SSL_alert_desc_string_long(ret));
    }
    else if (where & SSL_CB_EXIT)
    {
        if (ret == 0)
            fprintf(stderr,"%s:failed in %s\n",
                    str,SSL_state_string_long(s));
        else if (ret < 0)
        {
            fprintf(stderr,"%s:error in %s\n",
                    str,SSL_state_string_long(s));
        }
    }
}

SslSocket::SslSocket(std::weak_ptr<Kite::EventLoop> ev)
    : Kite::Evented(ev)
    , p(new SslSocketPrivate)
{
    static bool sslIsInit = false;
    if (!sslIsInit) {
        SSL_load_error_strings();
        SSL_library_init();
        ERR_load_BIO_strings();
        OpenSSL_add_all_algorithms();
    }
    p->ssl_ctx = 0;
    p->bio     = 0;
    p->ssl     = 0;
    p->state   = Disconnected;

    // init ssl context
    p->ssl_ctx = SSL_CTX_new(SSLv23_client_method());
    if (p->ssl_ctx == NULL)
    {
        ERR_print_errors_fp(stderr);
        return;
    }


//    SSL_CTX_set_info_callback(p->ssl_ctx, apps_ssl_info_callback);

}

SslSocket::~SslSocket()
{
    disconnect();


    if (p->bio)
        BIO_free_all(p->bio);

    if (p->ssl_ctx)
        SSL_CTX_free(p->ssl_ctx);

    delete p;
}

bool SslSocket::setCaDir (const std::string &path)
{
    int r = SSL_CTX_load_verify_locations(p->ssl_ctx, NULL, path.c_str());
    if (r == 0) {
        return false;
    }
    return true;
}

bool SslSocket::setCaFile(const std::string &path)
{
    int r = SSL_CTX_load_verify_locations(p->ssl_ctx, path.c_str(), NULL);
    if (r == 0) {
        return false;
    }
    return true;
}
bool SslSocket::setClientCertificateFile(const std::string &path)
{
    int r = SSL_CTX_use_certificate_file(p->ssl_ctx, path.c_str(), SSL_FILETYPE_PEM);
    if (r == 0) {
        return false;
    }
    return true;

}

bool SslSocket::setClientKeyFile(const std::string &path)
{
    int r = SSL_CTX_use_PrivateKey_file(p->ssl_ctx, path.c_str(), SSL_FILETYPE_PEM);
    if (r == 0) {
        return false;
    }
    return true;
}

void SslSocket::disconnect()
{
    int fd = 0;
    BIO_get_fd(p->bio, &fd);
    if (fd != 0)
        evRemove(fd);

    if (p->ssl) {
        auto e = rebind_map.find(p->ssl);
        if (e != rebind_map.end())
            rebind_map.erase(e);
    }

    if (p->bio)
        BIO_ssl_shutdown(p->bio);

    if (p->state == Connected || p->state == Connecting)
        p->state = Disconnected;
    onDisconnected(p->state);
}

void SslSocket::connect(const std::string &hostname, int port)
{
    p->state   = Connecting;
    int r = 0;

    p->bio = BIO_new_ssl_connect(p->ssl_ctx);
    if (!p->bio) {
        p->errorMessage = "BIO_new_ssl_connect returned NULL";
        p->state        = SslSetupError;
        disconnect();
        return;
    }

    BIO_get_ssl(p->bio, &p->ssl);
    if (!(p->ssl)) {
        p->errorMessage = "BIO_get_ssl returned NULL";
        p->state        = SslSetupError;
        disconnect();
        return;
    }

    rebind_map.insert(std::make_pair(p->ssl, this));
    auto cb = [](SSL *ssl, X509 **x509, EVP_PKEY **pkey) {
        SslSocket *that = rebind_map[ssl];
        that->p->errorMessage = "Client Certificate Requested\n";
        that->p->state = SslClientCertificateRequired;
        that->disconnect();
        return 0;
    };
    SSL_CTX_set_client_cert_cb(p->ssl_ctx, cb);


    SSL_set_mode(p->ssl, SSL_MODE_AUTO_RETRY);
    BIO_set_conn_hostname(p->bio, hostname.c_str());
    BIO_set_conn_int_port(p->bio, &port);

    r  = BIO_do_connect(p->bio);
    if (r < 1) {
        if (p->state != SslClientCertificateRequired) {
            const char *em = ERR_reason_error_string(ERR_get_error());
            fprintf(stderr, "BIO_new_ssl_connect failed: %lu (0x%lx)\n", r, r);
            fprintf(stderr, "Error: %s\n", em);
            fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
            fprintf(stderr,"p_ssl state: %s\n",SSL_state_string_long(p->ssl));
            ERR_print_errors_fp(stderr);
            p->errorMessage = em ? em : "??";
            p->state        = TransportErrror;
        }
        disconnect();
        return;
    }

    if (SSL_get_verify_result(p->ssl) != X509_V_OK) {
        p->errorMessage = "Ssl Peer Verification Errro";
        p->state        = SslPeerNotVerified;
        disconnect();
        return;
    }

    int fd = 0;
    BIO_get_fd(p->bio, &fd);
    if (fd == 0) {
        p->errorMessage = "BIO_get_fd returned 0";
        p->state        = SslSetupError;
        disconnect();
        return;
    }
    evAdd(fd);
    p->state        = Connected;
    onConnected();
}

ssize_t SslSocket::write(const char *data, int len)
{
    return BIO_write(p->bio, data, len);

}

ssize_t SslSocket::read (char *data, int len)
{
    return BIO_read(p->bio, data, len);
}

const std::string &SslSocket::errorMessage() const
{
    return p->errorMessage;
}

void SslSocket::flush()
{
    BIO_flush(p->bio);
}
