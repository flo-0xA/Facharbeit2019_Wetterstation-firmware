#ifndef INITIALIZATION_H

#define INITIALIZATION_H

#include <PubSubClient.h>

extern const int us_to_s_factor;
extern const int time_to_sleep_daytime;
extern const int time_to_sleep_nighttime;
extern const int time_to_sleep_lowpower;

extern PubSubClient mqtt_client;


void deepsleep_init();
bool interface_init();
bool wifi_init();
bool mqtt_init();

#endif