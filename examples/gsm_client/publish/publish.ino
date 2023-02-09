#include <TinyGsmClient.h>
#include "sim7600.h"

#define RXD2 27 // This is for ESP32 to 4G Module communication
#define TXD2 26
#define PWR_KEY 4        // Power Key to enable 4G Module
#define SerialAT Serial1 // Serial communication port between ESP32 and 4G module
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
Sim7600 sim7600_at;

void setup()
{
    Serial.begin(115200);
    SerialAT.begin(115200, SERIAL_8N1, RXD, TXD);

    Serial.println("Modem power On")
        digitalWrite(MODEM_PWKEY, LOW);
    delay(100);
    digitalWrite(MODEM_PWKEY, HIGH);
    delay(1000);
    digitalWrite(MODEM_PWKEY, LOW);

    sim7600_at.restartModem(&modem);
    delay(10000);
    sim7600_at.initialiseModem(&modem);
    sim7600_at.updateNTPTime(&modem);
    sim7600_at.configureSSL(&modem);

    if (!sim7600_at.startMQTTService(&modem))
    {
        Serial.println("Error Starting MQTT service");
    }
    if (!sim7600_at.accquireClient(&modem))
    {
        Serial.println("Error Accquiring client");
    }
    if (!sim7600_at.connectMQTT(&modem))
    {
        Serial.println("ERROR Connecting to MQTT");
    }
}

void loop()
{

    if (sim7600_at.connectMQTT(&modem))
    {
        Serial.println("client connected. Publish your meesage");
        sim7600_at.setMQTTTopic(&modem);
        sim7600_at.createMQTTPayload(&modem);
        sim7600_at.publishToTopic(&modem);
    }
    else
    {
        sim7600_at.releaseMQTTClient(&modem);
        sim7600_at.disconnectMQTTClient(&modem);
        sim7600_at.stopMQTTClient(&modem);
        sim7600_at.startMQTTService(&modem);
        sim7600_at.accquireClient(&modem);
        sim7600_at.connectMQTT(&modem);
        sim7600_at.setMQTTTopic(&modem);
        sim7600_at.createMQTTPayload(&modem);
        sim7600_at.publishToTopic(&modem);
    }
    delay(10000);
}
