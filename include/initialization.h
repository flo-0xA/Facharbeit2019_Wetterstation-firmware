#ifndef INITIALIZATION_H

#define INITIALIZATION_H

#include <PubSubClient.h>

extern PubSubClient mqtt_client;

void deepsleep_init();
bool interface_init();
bool wifi_init();
bool mqtt_init();

#endif