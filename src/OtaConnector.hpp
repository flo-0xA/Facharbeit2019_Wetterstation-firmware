#ifndef _OTACONNECTOR_HPP_

#define _OTACONNECTOR_HPP_

#include <WiFi.h>

namespace ota
{
    void init(WiFiClient client);
    bool exec();
}

#endif