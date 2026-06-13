#include "includes.h"
#include <Update.h>

bool updateFromFS();
bool performUpdate(Stream &updateSource, size_t updateSize);
void printPercent(uint32_t readLength, uint32_t contentLength);

HardwareSerial SerialGSM(1);
TinyGsm modemGSM(SerialGSM);
TinyGsmClient gsmClient(modemGSM);

String GSM_apn;
String GSM_login;
String GSM_pass;
String GSM_pin;

void GSM_init()
{
    GSM_apn = MEM_readString(ADR_GSM_APN);
    GSM_login = MEM_readString(ADR_GSM_LOGIN);
    GSM_pass = MEM_readString(ADR_GSM_PASS);
    GSM_pin = MEM_readString(ADR_GSM_PIN);

    DBG_tag("GSM_APN: ", GSM_apn);
    DBG_tag("GSM_LOGIN: ", GSM_login);
    DBG_tag("GSM_PASS: ", GSM_pass);
    DBG_tag("GSM_PIN: ", GSM_pin);

}
bool GSM_connect()
{
    DBG_PRINTLN(GSM_apn + " " + GSM_login + " " + GSM_pass);

    SerialGSM.begin(9600, SERIAL_8N1, PIN_RX_MODEM, PIN_TX_MODEM, false);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    esp_task_wdt_reset();

    //Inicializa o modem

    DBG_PRINTLN(modemGSM.getModemInfo());

    if (!modemGSM.restart())
    {
        digitalWrite(PIN_RST_MODEM, LOW);
        vTaskDelay(500 / portTICK_RATE_MS);
        digitalWrite(PIN_RST_MODEM, HIGH);

        DBG_PRINTLN("Restarting GSM Modem failed");
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    modemGSM.simUnlock(GSM_pin.c_str());

    esp_task_wdt_reset();
    //Espera pela rede

    if (!modemGSM.waitForNetwork())
    {
        DBG_PRINTLN("Fail to connect to network.");
        return false;
    }

    //Conecta à rede gprs (APN, usuário, senha)
    if (!modemGSM.gprsConnect(GSM_apn.c_str(), GSM_login.c_str(), GSM_pass.c_str()))
    {
        DBG_PRINTLN("Fail to connect to GPRS.");
        return false;
    }

    DBG_PRINTLN("Conected");
    return true;
}

bool GSM_isConected()
{
    return modemGSM.isGprsConnected() and modemGSM.isNetworkConnected();
}

int8_t GSM_getSignal()
{
    int16_t signal = modemGSM.getSignalQuality();
    if (signal < 32)
    {
        return (2*signal - 115);
    }
    return -1;
}

// OTA Logic
bool GSM_ota(String binName){
    DBG_PRINTLN("OTA fileName: " + binName);
    long contentLength = 0;
    bool isValidContentType = false;
    // Connect to S3
    if (gsmClient.connect(OTA_HOST, OTA_PORT))
    {
        // Connection Succeed.
        // Fecthing the bin

        // Get the contents of the bin file
        gsmClient.print(String("GET ") + binName + " HTTP/1.1\r\n" +
                        "Host: " + OTA_HOST + "\r\n" +
                        "Cache-Control: no-cache\r\n" +
                        "Connection: close\r\n\r\n");

        unsigned long timeout = millis();
        while (gsmClient.available() == 0)
        {
            if (millis() - timeout > 30000)
            {
                DBG_PRINTLN("Client Timeout !");
                gsmClient.stop();
                return false;
            }
        }
        while (gsmClient.available())
        {

            // read line till /n
            String line = gsmClient.readStringUntil('\n');
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
        return false;
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

            if (!SPIFFS.begin(true))
            {
                DBG_PRINTLN("SPIFFS Mount Failed");
                return false;
            }

            if(SPIFFS.exists("/update.bin"))
                deleteFile(SPIFFS,"/update.bin");

            File file = SPIFFS.open("/update.bin", FILE_WRITE);
            if (!file)
            {
                DBG_PRINTLN("Erro ao criar arquivo.");
                return false;
            }
            long readLength = 0;
            uint8_t buffer;
            while (readLength < contentLength && gsmClient.connected())
            {
                while (gsmClient.available())
                {
                    buffer = gsmClient.read();
                    if (!file.write(buffer))
                    {
                        DBG_PRINTLN("Fallo Append");
                    }
                    readLength++;
                }
                esp_task_wdt_reset();
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }

            DBG_PRINTLN("ContentLength: " + String(contentLength));
            DBG_PRINTLN("ReadLength: " + String(readLength));
            DBG_PRINTLN("FileLength: " + String(file.size()));

            file.close();

            gsmClient.stop();

            return updateFromFS();
        }
    }
    return false;
}
bool updateFromFS()
{
    File updateBin = SPIFFS.open("/update.bin");
    if (updateBin)
    {
        if (updateBin.isDirectory())
        {
            DBG_PRINTLN("Error, en el directorio");
            updateBin.close();
            return false;
        }

        size_t updateSize = updateBin.size();
        DBG_PRINTLN("Tamanho: " + String(updateSize));

        if (updateSize > 0)
        {
            DBG_PRINTLN("Intentando comenzar Actualización");
            performUpdate(updateBin, updateSize);
        }
        else
        {
            DBG_PRINTLN("Error, archivo vacío");
        }

        updateBin.close();
    }
    else
    {
        DBG_PRINTLN("No se puede cargar el archivo");
    }
    return false;
}

bool performUpdate(Stream &updateSource, size_t updateSize)
{
    DBG_PRINTLN("Tamanho: " + String(updateSize));
    size_t written = Update.writeStream(updateSource);
    if (written == updateSize)
    {
        DBG_PRINTLN("Escritos : " + String(written) + " successfully");
    }
    else
    {
        DBG_PRINTLN("Solamente escritos : " + String(written) + "/" + String(updateSize) + ". Retry?");
    }
    if (Update.end())
    {
        DBG_PRINTLN("OTA realizado!");
        if (Update.isFinished())
        {
            DBG_PRINTLN("Ota exitoso, reiniciando!");
            return true;
        }
        else
        {
            DBG_PRINTLN("Ota no terminó? Algo salió mal!");
        }
    }
    else
    {
        DBG_PRINTLN("Ocurrió Error #: " + String(Update.getError()));
    }
    return false;
}

void printPercent(uint32_t readLength, uint32_t contentLength)
{
    // If we know the total length
    if (contentLength != -1)
    {
        DBG_PRINT("\r ");
        DBG_PRINT((100.0 * readLength) / contentLength);
        DBG_PRINT('%');
    }
    else
    {
        DBG_PRINTLN(readLength);
    }
}