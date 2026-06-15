
#include <SimpleCLI.h>
#include "includes.h"

TaskHandle_t CLI_handleTask;

void getInfoCallback(cmd *commandPointer);
void setCallback(cmd *commandPointer);
void helpCallback(cmd * command);
void rebootCallback(cmd * command);
void relayScanCallback(cmd * command);
void relayPinCallback(cmd * command);

void errorCallback(cmd_error *e);

// Create CLI Object
SimpleCLI cli;

// Commands
Command getInfo;
Command set;
Command help;
Command reboot;
Command relayScan;
Command relayPin;

void CLI_task(void *pvt)
{
    esp_task_wdt_delete(MQTT_handleTask);
    vTaskDelete(MQTT_handleTask);
    for (size_t i = 0; i < MAX_PINS_INPUT; i++)
    {
        detachInterrupt(PINS_ENTRY[i]);
    }


    Serial.begin(9600);

    vTaskDelay(2000 / portTICK_PERIOD_MS);
    cli.setOnError(errorCallback);


    getInfo = cli.addCmd("show", getInfoCallback);
    set = cli.addCmd("set", setCallback);
    set.addArg("i");
    set.addFlagArg("r");
    help = cli.addCmd("help", helpCallback);
    reboot = cli.addCmd("reboot", rebootCallback);
    relayScan = cli.addCmd("relayscan", relayScanCallback);
    relayPin = cli.addCmd("relaypin", relayPinCallback);
    relayPin.addArg("p");

    for (;;)
    {
        Serial.print("esp-cli: # ");
        String message;
        while (!Serial.available())
            vTaskDelay(100 / portTICK_PERIOD_MS);
        char buffer = Serial.read();
        if (!(buffer == 8))
        { // backspace

            message.concat(buffer);
            Serial.print(buffer);
            do
            {
                if (Serial.available())
                {
                    buffer = Serial.read();
                    if (buffer == 8)
                    { //backspace
                        message.remove(message.length() - 1);
                    }
                    else
                    {
                        message.concat(buffer);
                    }
                    Serial.print(buffer);
                }
                vTaskDelay(10 / portTICK_PERIOD_MS);
            } while (buffer != '\n');
        }
        // Read out string from the serial monitor

        cli.parse(message);
    }
}

void getInfoCallback(cmd *commandPointer)
{
    Serial.println("============== Black Telemetry ==============\n");

    Serial.print("Device ID: TEL-");
    Serial.println(MEM_readUShort(ADR_DEVICE_ID));
    Serial.print("Version: ");
    Serial.println(FW_VERSION);
    Serial.print("Network: ");
    eREDE net = (eREDE)MEM_readChar(ADR_REDE);
    if (net == GSM)
        Serial.println("GSM");
    else if (net == WIFI)
        Serial.println("WiFi");
    else
        Serial.println("Undefined");
    Serial.println("GSM ");
    Serial.print("  APN: ");
    Serial.println(MEM_readString(ADR_GSM_APN));
    Serial.print("  Login: ");
    Serial.println(MEM_readString(ADR_GSM_LOGIN));
    Serial.print("  Pass: ");
    Serial.println(MEM_readString(ADR_GSM_PASS));
    Serial.print("  PIN: ");
    Serial.println(MEM_readString(ADR_GSM_PIN));
    Serial.println("WiFi");
    Serial.print("  SSID: ");
    Serial.println(MEM_readString(ADR_WIFI_SSID));
    Serial.print("  Pass: ");
    Serial.println(MEM_readString(ADR_WIFI_PSW));
    Serial.println();
    Serial.println("=====================================\n");
}

void setCallback(cmd *command)
{
    Command c(command);
    Argument r = c.getArg("r");
    Argument i = c.getArg("i");

    if (!i.isSet() || i.getValue().length() == 0)
    {
        Serial.println("ERROR: missing device id. Use: set -i <id> [-r]");
        return;
    }

    uint16_t deviceId = static_cast<uint16_t>(i.getValue().toInt());
    if (deviceId == 0)
    {
        Serial.println("ERROR: invalid device id");
        return;
    }

    MEM_writeUShort(ADR_DEVICE_ID, deviceId);
    if (r.isSet())
    {
        MEM_writeString(ADR_GSM_APN, "voxter.br");
        MEM_writeString(ADR_GSM_LOGIN, "algar");
        MEM_writeString(ADR_GSM_PASS, "algar");
        MEM_writeString(ADR_GSM_PIN, "1212");
        MEM_writeChar(ADR_REDE, 0);
        MEM_writeString(ADR_WIFI_SSID, "");
        MEM_writeString(ADR_WIFI_PSW, "");
        PinInput_t buffer;
        buffer.pinNumber = 0;
        buffer.qtd = 0;
        for (size_t i = 0; i < MAX_PINS_INPUT; i++)
        {
            MEM_writePinInput(ADR_PINS + i * MAX_SIZE_PIN_INPUT, buffer );
        }
        MEM_writeUShort(ADR_DATA_OFFLINE, 0);
        
    }
}

void errorCallback(cmd_error *e)
{
    CommandError cmdError(e); // Create wrapper object

    // Print error
    Serial.print("ERROR: ");
    Serial.println(cmdError.toString());

    // Print command usage
    if (cmdError.hasCommand())
    {
        Serial.printf("Valid command: ");
        Serial.println(cmdError.getCommand().toString());
    }
}

void helpCallback(cmd * command){
    Serial.println("Supported commands: ");
    Serial.println(cli.toString());
}

void rebootCallback(cmd * command){
    Serial.println("Rebooting...");
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    ESP.restart();
}

void relayScanCallback(cmd * command)
{
    RELAY_scan();
}

void relayPinCallback(cmd *command)
{
    Command c(command);
    Argument p = c.getArg("p");
    if (!p.isSet() || p.getValue().length() == 0)
    {
        Serial.println("ERROR: missing pin. Use: relaypin -p <gpio>");
        return;
    }

    int pin = p.getValue().toInt();
    if (pin < 0 || pin > 39)
    {
        Serial.println("ERROR: invalid gpio");
        return;
    }

    RELAY_setPin((uint8_t)pin);
    Serial.printf("Relay pin updated to %d\n", pin);
}
