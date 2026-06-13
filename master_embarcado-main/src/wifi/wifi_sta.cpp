#include "includes.h"
#include "Update.h"

WiFiClient wifiClient;
String WIFI_ssid;
String WIFI_psw;

void WIFI_init(void)
{
    WIFI_ssid = MEM_readString(ADR_WIFI_SSID);
    WIFI_psw = MEM_readString(ADR_WIFI_PSW);

    DBG_tag("SSID: ", WIFI_ssid);
    DBG_tag("PSW: ", WIFI_psw);

}

bool WIFI_connect()
{

    WiFi.disconnect(true);

    vTaskDelay(300 / portTICK_PERIOD_MS);

    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_ssid.c_str(), WIFI_psw.c_str());

    uint8_t tentativas = 0;

    while (WiFi.status() != WL_CONNECTED and tentativas < 20)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        tentativas++;
        esp_task_wdt_reset();
    }
    return tentativas < 20; // Se for menor que 20, wifi está conectado.
}

bool WIFI_isConected()
{
    if (WiFi.status() != WL_CONNECTED)
        return false;
    return true;
}

String getHeaderValue(String header, String headerName)
{
    return header.substring(strlen(headerName.c_str()));
}

bool WIFI_ota(String binName)
{
    DBG_PRINTLN("OTA fileName: " + binName);
    long contentLength = 0;
    bool isValidContentType = false;
    // Connect to S3
    if (wifiClient.connect(OTA_HOST, OTA_PORT))
    {
        // Connection Succeed.
        // Fecthing the bin

        // Get the contents of the bin file
        wifiClient.print(String("GET ") + binName + " HTTP/1.1\r\n" +
                         "Host: " + OTA_HOST + "\r\n" +
                         "Cache-Control: no-cache\r\n" +
                         "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (wifiClient.available() == 0)
        {
            if (millis() - timeout > 30000)
            {
                DBG_PRINTLN("Client Timeout !");
                wifiClient.stop();
                return false;
            }
        }
        while (wifiClient.available())
        {
            // read line till /n
            String line = wifiClient.readStringUntil('\n');
            DBG_PRINTLN(line);
            // remove space, to check if the line is end of headers
            line.trim();

            // if the the line is empty,
            // this is end of headers
            // break the while and feed the
            // remaining `client` to the
            // Update.writeStream();
            if (!line.length())
            {
                //headers ended
                break; // and get the OTA started
            }

            // Check if the HTTP Response is 200
            // else break and Exit Update
            if (line.startsWith("HTTP/1.1"))
            {
                if (line.indexOf("200") < 0)
                {
                    DBG_PRINTLN("Got a non 200 status code from server. Exiting OTA Update.");
                    break;
                }
            }

            // extract headers here
            // Start with content length
            if (line.startsWith("Content-Length: "))
            {
                contentLength = atol(line.substring(strlen("Content-Length: ")).c_str());

                DBG_PRINTLN("Got " + String(contentLength) + " bytes from server");
            }

            // Next, the content type
            if (line.startsWith("Content-Type: "))
            {
                String contentType = line.substring(strlen("Content-Type: "));
                DBG_PRINTLN("Got " + contentType + " payload.");
                if (contentType == "application/octet-stream")
                {
                    isValidContentType = true;
                }
            }
        }
    }
    else
    {
        // Connect to S3 failed
        // May be try?
        // Probably a choppy network?
        DBG_PRINTLN("Connection to " + String(OTA_HOST) + " failed. Please check your setup");
        // retry??
        // execOTA();
    }

    // Check what is the contentLength and if content type is `application/octet-stream`
    DBG_PRINTLN("contentLength : " + String(contentLength) + ", isValidContentType : " + String(isValidContentType));

    // check contentLength and content type
    if (contentLength && isValidContentType)
    {
        // Check if there is enough to OTA Update
        bool canBegin = Update.begin(contentLength);

        // If yes, begin
        if (canBegin)
        {
            DBG_PRINTLN("Begin OTA. This may take 2 - 5 mins to complete. Things might be quite for a while.. Patience!");
            // No activity would appear on the Serial monitor
            // So be patient. This may take 2 - 5mins to complete
            size_t written = Update.writeStream(wifiClient);

            if (written == contentLength)
            {
                DBG_PRINTLN("Written : " + String(written) + " successfully");
            }
            else
            {
                DBG_PRINTLN("Written only : " + String(written) + "/" + String(contentLength) + ". Retry?");
                // retry??
                // execOTA();
            }

            if (Update.end())
            {
                DBG_PRINTLN("OTA done!");
                if (Update.isFinished())
                {
                    DBG_PRINTLN("Update successfully completed. Rebooting.");
                    return true;
                }
                else
                {
                    DBG_PRINTLN("Update not finished? Something went wrong!");
                }
            }
            else
            {
                DBG_PRINTLN("Error Occurred. Error #: " + String(Update.getError()));
            }
        }
        else
        {
            // not enough space to begin OTA
            // Understand the partitions and
            // space availability
            DBG_PRINTLN("Not enough space to begin OTA");
        }
    }
    else
    {
        DBG_PRINTLN("There was no content in the response");
    }
    return false;
}