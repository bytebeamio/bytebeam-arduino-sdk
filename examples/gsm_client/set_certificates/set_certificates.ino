#include <TinyGsmClient.h>
#include "certs.h"

#define RXD2 27 // This is for ESP32 to 4G Module communication
#define TXD2 26
#define PWR_KEY 4        // Power Key to enable 4G Module
#define AT_WAIT_TIME_MSECS 12000L //Max Wait time for AT
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
    if(setCaCerts() ? Serial.println("CA certs uploaded to SIM7600" : Serial.println("CA Certificate set failed")));
    if(setClientCerts() ? Serial.println("Client certs uploaded to SIM7600" : Serial.println("Client Certificate set failed")));
    if(setClientKey() ? Serial.println("Client Key certs uploaded to SIM7600" : Serial.println("CA Certificate set failed")));

}

void loop()
{

}

void restartModem()
{
    modem->restart();
    String modemInfo = modem->getModemInfo();
    Serial.printf("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
}


bool setClientCerts() {
  const int client_cert_size = sizeof(clientcert);

  modem.sendAT(GF("+CCERTDOWN=\""CLIENTCERT_FILE_NAME"\""), client_cert_size);
  if (modem.waitResponse(GF(">")) != 1) {
    return false;
  }

  for (int i = 0; i < client_cert_size; i++) {
    
    char c = pgm_read_byte(&clientcert[i]);
    modem.stream.write(c);
  }
  modem.stream.flush();
  modem->waitResponse(AT_WAIT_TIME_MSECS);
}
  
  
bool setCaCerts(){

  const int ca_cert_size = sizeof(cacert);
  modem.sendAT(GF("+CCERTDOWN=\""CACERT_FILE_NAME"\""), ca_cert_size);
  if (modem.waitResponse(GF(">")) != 1) {
    return false;
  }

  for (int i = 0; i < ca_cert_size; i++) {
    char c = pgm_read_byte(&cacert[i]);
    modem.stream.write(c);
  }

  modem.stream.flush();
  modem->waitResponse(AT_WAIT_TIME_MSECS);

}


bool setClientKey(){
  const int key_cert_size = sizeof(clientkey);
  modem.sendAT(GF("+CCERTDOWN=\""CLIENTKEY_FILE_NAME"\""), key_cert_size);
  if (modem.waitResponse(GF(">")) != 1) {
    return false;
  }
  for (int i = 0; i < ca_cert_size; i++) {
    char c = pgm_read_byte(&clientkey[i]);
    modem.stream.write(c);
  }

  modem.stream.flush();
  modem->waitResponse(AT_WAIT_TIME_MSECS);
}
