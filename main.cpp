#include <iostream>

#include "Kite/EventLoop.hpp"
#include "Kite/MqttClient.hpp"


class MyClient : public Kite::MqttClient
{
public:
    MyClient(std::weak_ptr<Kite::EventLoop> ev)
        : Kite::MqttClient(ev)
    {
    }
protected:
    virtual void onPublished (const std::string &topic, const std::string &message) {
        fprintf(stderr, "message on %s: %s\n", topic.c_str(), message.c_str());
    }
    virtual void onMqttConnected() {
        fprintf(stderr, "omg MQTT is totally connected lol \n");
        subscribe("warf");
        publish("warf", "blurp");
    }
    virtual void onDisconnected(SocketState state) {
        fprintf(stderr, "disconnected: %i %s \n", state, errorMessage().c_str());
        ev()->exit(9);
    }
};

int main(int argc, char **argv)
{
    std::shared_ptr<Kite::EventLoop> ev(new Kite::EventLoop);
    std::shared_ptr<MyClient>        client(new MyClient(ev));

    client->setClientId(argv[1]);
    client->setCaFile("ca.crt");
    client->setClientCertificateFile("client.crt");
    client->setClientKeyFile("client.key");
    client->connect("e1.ep1.airfy.com", 8883);

    return ev->exec();
}
