#include <stdio.h>
#include <string>

#include "Kite/EventLoop.hpp"
#include "Kite/SslSocket.hpp"


class MySocket: public Kite::SslSocket
{
public:
    MySocket(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::SslSocket(ev)
    {
    }

protected:
    virtual void onActivated(int)
    {
        char buf [333];
        int r = read(buf, 100);
        buf[r] = 0;
        fprintf(stderr, "%s", buf);
        if (r == 0) {
            fprintf(stderr, "EOF\n");
            ev()->exit(0);
        }
    }
    virtual void onConnected() {
        fprintf(stderr, "connected!\n");
    }
    virtual void onDisconnected(SocketState state) {
        fprintf(stderr, "disconnected: %i %s \n", state, errorMessage().c_str());
        ev()->exit(9);
    }
};


int main()
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MySocket>        sock(new MySocket(ev));

    sock->setCaFile("ca.crt");
    sock->setClientCertificateFile("client.crt");
    sock->setClientKeyFile("client.key");
    sock->connect("e1.ep1.airfy.com", 8883);

//    int r;
//    r = sock->write("GET /\n", 6);


    return ev->exec();
}
