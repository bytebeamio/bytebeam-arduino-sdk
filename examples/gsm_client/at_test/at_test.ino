#include <TinyGsmClient.h>

#define RXD2 27 // This is for ESP32 to 4G Module communication
#define TXD2 26
#define PWR_KEY 4        // Power Key to enable 4G Module
#define SerialAT Serial1 // Serial communication port between ESP32 and 4G module
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

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

    restartModem();

}

void loop()
{
      if (Serial.available() > 0) // read AT commands from user Serial port and send to the Module
  {
    a = Serial.read();
    SerialAT.write(a);
  }
  if (SerialAT.available() > 0) //read Response commands from module and send to user Serial Port
  {
    b = SerialAT.read();
    Serial.write(b);
  }
}

/**
          @brief: restart Modem
    */

void restartModem()
{
    modem->restart();
    String modemInfo = modem->getModemInfo();
    Serial.printf("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
}
