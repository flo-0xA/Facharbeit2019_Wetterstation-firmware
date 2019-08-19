#ifndef _WLANCONNECTOR_HPP_

#define _WLANCONNECTOR_HPP_

#include <WiFi.h>

namespace wlan 
{
    void connect();
    void disconnect();
    bool isConnected();
    WiFiClient& getClient();
}

#endif