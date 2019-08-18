#ifndef _OTACONNECTOR_HPP_

#define _OTACONNECTOR_HPP_

#include <WiFi.h>

namespace ota
{
//  String getHeaderValue(String header, String headerName);
    void init(WiFiClient client);
    void exec();
}

#endif