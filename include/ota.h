#ifndef __OTA_H__
#define __OTA_H__

#include <ArduinoOTA.h>

TaskHandle_t *task = NULL;

class OTA
{
    public:
        void begin(int port, const char* password);
        void start();
        void stop();
};

void OTA::begin(int port, const char* password)
{
    ArduinoOTA.setPort(port);
    ArduinoOTA.setPassword(password);

    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })

    .onEnd([]() {
      Serial.println("\nEnd");
    })

    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("OTA: progress: %u%%\r", (progress / (total / 100)));
    })

    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("OTA: auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("OTA: begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("OTA: connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA: receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("OTA: end Failed");
    });
}

void OTA::start()
{
    xTaskCreate(
        start_task,
        "start",
        1000,
        NULL,
        1,
        task
    );
}

void OTA::stop()
{
    vTaskDelete(task);
}

void start_task(void *p)
{
   ArduinoOTA.handle();    
}

#endif