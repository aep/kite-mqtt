#ifndef KITE_SSLSOCKET_HPP_KNMS
#define KITE_SSLSOCKET_HPP_KNMS

#include <string>
#include "EventLoop.hpp"
#include "IO.hpp"

namespace Kite {
    class SslSocketPrivate;
    class SslSocket : public Kite::Evented, public Kite::IO
    {
    public:
        enum SocketState {
            Disconnected       = 1,
            Connecting         = 2,
            Connected          = 3,
            SslSetupError      = 4,
            SslPeerNotVerified = 5,
            SslClientCertificateRequired = 6,
            TransportErrror    = 7
        };

        SslSocket(std::weak_ptr<Kite::EventLoop>);
        ~SslSocket();

        bool setCaDir (const std::string &path);
        bool setCaFile(const std::string &path);

        bool setClientCertificateFile(const std::string &path);
        bool setClientKeyFile(const std::string &path);

        SocketState state() const;
        void connect(const std::string &hostname, int port);
        void disconnect();

        ssize_t write(const char *data, int len);
        ssize_t read (char *data, int len);
        void flush();

        const std::string &errorMessage() const;
    protected:
        virtual void onConnected(){}
        virtual void onDisconnected(SocketState state){}
    private:
        SslSocketPrivate *p;
    };
};

#endif
