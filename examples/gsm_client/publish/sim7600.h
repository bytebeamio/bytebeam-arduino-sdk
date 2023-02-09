#ifndef SIM7600_H
#define SIM7600_H

#define JSON_MSG_MAX_LEN 512
#define TINY_GSM_MODEM_SIM7600
#define TINY_GSM_RX_BUFFER 1024
#include <TinyGsmClient.h>
#define AT_WAIT_TIME_MSECS 12000L //Max Wait time for AT

#define CLIENTCERT_FILE_NAME    "clientcert.pem"
#define CACERT_FILE_NAME        "cacert.pem"
#define CLIENTKEY_FILE_NAME     "clientkey.pem"
#define NTP_SERVER              "pool.ntp.org"
#define MQTT_HOST               "tcp://<your_broker_url>:8883"
#define MQTT_CLIENT_NAME        "Add client name here"    
const char topic_publish[] =    "Add your publish topic here";
const char topic_subscribe[] =  "Add your subscribe topic here"


const int port = 8883;
  


class Sim7600
{
  
  public:
    /**
       @brief: Set MQTT topic to publish
       @return: true when everything works right
    */

    bool setMQTTTopic(TinyGsm *modem) {
      modem->sendAT(GF("+CMQTTTOPIC=0,21"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
      size_t topic_size = strlen(topic_publish);
      modem->stream.write(topic_publish, topic_size);
      modem->stream.write(GSM_NL);
      modem->stream.flush();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
      return res;
    }
    
    /**
       @brief: Creat MQTT payload to publish
       @return: true when everything works right
    */
    
    bool createMQTTPayload(TinyGsm *modem, String payload){
      size_t payload_size = strlen(payload.c_str());
      modem->sendAT(GF("+CMQTTPAYLOAD=0"),',',payload_size);
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
      modem->stream.write(payload.c_str(),payload_size);
      modem->stream.write(GSM_NL);
      modem->stream.flush();
      modem->waitResponse(AT_WAIT_TIME_MSECS);
      return true;
    }

    /**
       @brief: Publish to aws topic
       @return: true when everything works right
    */

    bool publishToTopic(TinyGsm *modem)
    {
      Serial.println("MQTT publishing message");
      modem->sendAT(GF("+CMQTTPUB=0,1,60"));
      modem->waitResponse();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTPUB: 0,0"));
     
      return res;
    }

    /**
       @brief:  Input subscribe topic
       @return: true when everything works right
    */

    bool subscribeToTopic(TinyGsm *modem,int qos) {
      size_t topic_size = strlen(topic_subscribe);
      modem->sendAT(GF("+CMQTTSUB=0"),',',topic_size,',',qos);
      if (modem->waitResponse(AT_WAIT_TIME_MSECS,GF(">")) != 1) return false;
      modem->stream.write(topic_subscribe, topic_size);
      modem->stream.write(GSM_NL);
      modem->stream.flush();
      modem->waitResponse(AT_WAIT_TIME_MSECS);
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTSUB: 0,0"));
      return res;
    }

    
    /**
        @brief: Configure SSL Configuration
        @return: true when everything goes write
    */

    bool configureSSL(TinyGsm *modem) {
      Serial.println("Configure SSL version");
      modem->sendAT(GF("+CSSLCFG=\"sslversion\",0,4"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
      
      Serial.println("Configure Auth mode");
      modem->sendAT(GF("+CSSLCFG=\"authmode\",0,2"));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
  
      Serial.println("Configure SSL cacert");
      modem->sendAT(GF("+CSSLCFG=\"cacert\",0,\"" CACERT_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
     
      Serial.println("Configure SSL clientcert");
      modem->sendAT(GF("+CSSLCFG=\"clientcert\",0,\"" CLIENTCERT_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
  
      Serial.println("Configure SSL clientkey");
      modem->sendAT(GF("+CSSLCFG=\"clientkey\",0,\"" CLIENTKEY_FILE_NAME "\""));
      if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;

      return true;
    }


    /**
        @brief: Start MQTT service
        @return: true when everything goes write
    */

   bool startMQTTService(TinyGsm *modem){
    Serial.println("Starting MQTT");
    modem->sendAT(GF ("+CMQTTSTART"));
    modem->waitResponse(AT_WAIT_TIME_MSECS);
    int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTSTART:"));
    if(!res){
        Serial.println("Could not find +CMQTTSTART");
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
      }
    Serial.println("Set SSL Configuration");
    modem->sendAT(GF ("+CMQTTSSLCFG=0,0"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
    clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_StartMqttFailed);
    return true;
   }

    /**
        @brief: Accquire MQTT Client
        @return: true when everything goes write
    */
   bool accquireClient(TinyGsm *modem){
     modem->sendAT(GF("+CMQTTACCQ=0,\"" MQTT_CLIENT_NAME "\",1"));
    if (modem->waitResponse(AT_WAIT_TIME_MSECS) != 1) return false;
    return true;
   }

    /**
        @brief: make MQTT Connection
        @return: true when everything goes write
    */
    bool connectMQTT(TinyGsm *modem) {
      modem->sendAT(GF("+CMQTTCONNECT=0,\"" MQTT_HOST "\",90,1"));
      modem->waitResponse();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTCONNECT: 0,0"));
      if (!res){
        setBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      }else{
        clearBit(RSTATE.deviceEvents, DeviceStateEvent::DSE_ConnectMqttFailed);
      }
      return res;
    }

    /**
        @brief: release MQTT Client
        @return: true when everything goes write
    */
    bool releaseMQTTClient(TinyGsm *modem) {
      Serial.println("RElease Mqtt Client");
      modem->sendAT(GF("+CMQTTREL?"));
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
      return res;
    }

    /**
        @brief: Open GSN Network
        @return: true when everything goes write
    */
    
    bool openNetwork(TinyGsm *modem){
        modem->sendAT(GF("+NETOPEN"));
        if (modem->waitResponse(AT_WAIT_TIME_MSECS, ("+NETOPEN: 0")) != 1) return false;
        return true;
    }

    /**
        @brief: Stop mqtt service and stop connection
        @return: true when everything goes write
    */
    bool stopMQTTClient(TinyGsm *modem) {
      Serial.println("Stop Mqtt Client");
      modem->sendAT(GF("+CMQTTSTOP"));
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
      return res;
    }

    /**
        @brief: disconnect mqtt connection
        @param: TinyGsm pointer
        @return: true when everything goes write
    */
    bool disconnectMQTTClient(TinyGsm *modem) {
      Serial.println("Stop Mqtt Client");
      modem->sendAT(GF("+CMQTTDISC=0,120"));
      modem->waitResponse();
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS,GF(GSM_NL "+CMQTTDISC: 0,0"));
      return res;
    }


    /**
          @brief: Create message payload
          @param: Sensor profile of sesnor type
          @return: message payload array
    */
   bool updateNTPTime(TinyGsm *modem){
      Serial.println("Update NTP Time");
      modem->sendAT("+CNTP=\"" NTP_SERVER "\",32");
      int res = modem->waitResponse(AT_WAIT_TIME_MSECS);
      return res;
   }


    /**
          @brief: restart Modem
          @param: TinyGsm pointer
          @return: true when everything goes write
    */

    void restartModem(TinyGsm *modem)
    {
      modem->restart();
      String modemInfo = modem->getModemInfo();
      Serial.printf("modemInfo: %s\n SimStatue: %d\n", modemInfo.c_str(), modem->getSimStatus());
    }


    /**
          @brief: Initialise Modem
          @param: TinyGsm pointer
          @return: true when everything goes write
    */

    void initialiseModem(TinyGsm *modem)
    { Serial.println("initialising Modem");
      modem->init();
      String modemInfo = modem->getModemInfo();
      Serial.printf("modemInfo: %s\n SimStatus: %d\n", modemInfo.c_str(), modem->getSimStatus());
    }

    
};

#endif