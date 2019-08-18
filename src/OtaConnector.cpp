/*
 * 
 * OTA-Funktionalität von: https://github.com/espressif/arduino-esp32/issues/325
 */
#include "OtaConnector.hpp"

#include "GlobalDefinitions.hpp"

#include <WiFi.h>
#include <Update.h>

namespace ota
{
    WiFiClient client;

    int returnValue = 0;

    int contentLength = 0;
    bool isValidContentType = false;
    String contentType = "";

    String getHeaderValue(String header, String headerName)
    {
        return header.substring(strlen(headerName.c_str()));
    }

    void init(WiFiClient &obj)
    {
        client = obj;
    }

    bool exec()
    {
        Serial.println("Verbinden mit: " + String(OTA_HOST));

        if (client.connect(String(OTA_HOST).c_str(), OTA_PORT))
        {
            Serial.println("Firmware holen: " + String(OTA_FIRMAWARE));

            client.print(String("GET ") + OTA_FIRMAWARE + " HTTP/1.1\r\n" +
                    "Host: " + OTA_HOST + "\r\n" +
                    "Cache-Control: no-chace\r\n" +
                    "Connection: close\r\n\r\n"
            );

            unsigned long timeout = millis();
            while (client.available() == 0)
            {
                if (millis() - timeout > 5000)
                {
                    Serial.println("Client Zeitüberschreitung!");
                    client.stop();
                    return;
                }
            }

            while (client.available())
            {
                String line = client.readStringUntil('\n');     // Zeile auslesen bis Ende
                line.trim();    // Leerzeichen entfernen

                Serial.println(line);

                if (!line.length())
                {
                    break;  // OTA starten
                }

                if (line.startsWith("HTTP/1.1"))
                {
                    if (line.indexOf("200") < 0)
                    {
                        Serial.println("200-Status-Code vom Server erhalten. OTA-Update wird abgebrochen.");
                        break;
                    }
                }

                if (line.startsWith("Content-Length: "))
                {
                    contentLength = atoi((getHeaderValue(line, "Content-Length: ")).c_str());
                    Serial.println(String(contentLength) + "Bytes vom Server erhalten");
                
                    if (contentType == "application/octet-stream" || contentType == "application/macbinary")
                    {
                        isValidContentType = true;
                    }
                }
            }

        } 
        else
        {
            // Verbindung zum Server ist fehlgeschlagen
            Serial.println("Verbindung zu: " + String(OTA_HOST) + " ist fehlgeschlagen. Verbindungsdaten prüfen.");
        }

        Serial.println(contentType + "geholt.");
        Serial.println(String(contentLength) + " Bytes vom Server geholt");

        Serial.println("contentLength: " + String(contentLength) + ", isValidContentType: " + String(isValidContentType));

        if (contentLength && isValidContentType)
        {
            bool canBegin = Update.begin(contentLength);

            if (canBegin)
            {
                Serial.println("OTA-Update wird gestartet. Dies kann 2 - 5 Minuten andauern. Bitte warten!");

                size_t written = Update.writeStream(client);

                if (written == contentLength)
                {
                    Serial.println(String(written) + "erfolgreich geschrieben");
                }
                else
                {
                    Serial.println("Nur " + String(written) + "/" + String(contentLength) + " geschrieben. Fehlgeschlagen!");
                }

                if (Update.end())
                {
                    Serial.print("OTA-Update fertig!");

                    if (Update.isFinished())
                    {
                        Serial.println("Update wurde erfolgreich ausgeführt! Neustart wird ausgeführt.");
                        ESP.restart();
                    }
                    else
                    {
                        Serial.println("Update nicht fertiggestellt? Ein Fehler ist aufgetreten!");
                        
                        returnValue = -1;
                    }
                }
                else
                {
                    Serial.println("Ein Fehler ist aufgetreten. Fehler #: " + String(Update.getError()));
                
                    returnValue = -1;
                }
            } 
            else 
            {
                Serial.println("Nicht genug Speicher verfügbar!");
                client.flush();
            
                returnValue = -1;
            }
        }
        else 
        {
            Serial.println("Kein Inhalt in Antwort gefunden");
            client.flush();
        
            returnValue = -1;
        }

        return returnValue;
    }
}