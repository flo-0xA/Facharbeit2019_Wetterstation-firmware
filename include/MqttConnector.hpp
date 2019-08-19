#ifndef _MQTTCONNECTOR_HPP_

#define _MQTTCONNECTOR_HPP_

namespace mqtt
{
    void connect();
    void disconnect();
    bool isConnected();
    bool publish(char* topic, const char* message);
}

#endif