#ifndef _WLANCONNECTOR_HPP_

#define _WLANCONNECTOR_HPP_

#include <WiFi.h>

namespace wlan 
{
    void connect();
    bool isConnected();
    WiFiClient& getClient();
}

#endif