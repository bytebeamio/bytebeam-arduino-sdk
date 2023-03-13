#ifndef DEVICESTATE_H
#define DEVICESTATE_H

#include <EEPROM.h>
#include "hardwareDefs.h"
#include "LinkedList.h" // arduino library by evan seidel
#include "SensorPayload.h"



//advance declaration
class PersistantStateStorageFormat;


/**
   @brief:
   Class for runtime Device status
*/
class RunTimeState {
  public:
    RunTimeState():
      isWiFiConnected(false),
      isAPActive(false),
      macAddr(DEVICE_ID_DEFAULT)
    {

    }
    bool isWiFiConnected;
    bool isAPActive;
    String macAddr;
};

/**
   @brief:
   Class EEPROM device format
*/

class PersistantState {
  public:
    PersistantState() : 
    apSSID(WAN_WIFI_SSID_DEFAULT),
     apPass(WAN_WIFI_PASS_DEFAULT)
    {

    }
    PersistantState(const PersistantStateStorageFormat& persistantStore);

    bool operator==(const PersistantState& rhs) {
      return ((apSSID == rhs.apSSID) &&
              (apPass == rhs.apPass));
    }
    String apSSID;
    String apPass;
};

/**
   @brief:
   Structure EEPROM Storage format
   this shadwos persistnat state structure in every way except that
   it replaces complex data types with POD types, complex data can't be directly stored and
   read back as is. It was required because we don't want to deal with c strings in rest of the code.
*/

struct PersistantStateStorageFormat {
  public:
    PersistantStateStorageFormat() {}
    PersistantStateStorageFormat(const PersistantState &persistantState);
    char version[8];
    char apSSID[30];
    char apPass[30];
} __attribute__ ((packed));

PersistantState::PersistantState(const PersistantStateStorageFormat& persistantStore)
{
  apSSID = String(persistantStore.apSSID);
  apPass = String(persistantStore.apPass);
}

PersistantStateStorageFormat::PersistantStateStorageFormat(const PersistantState &persistantState)
{
  strcpy(version, EEPROM_STORAGE_FORMAT_VERSION);
  strcpy(apSSID, persistantState.apSSID.c_str());
  strcpy(apPass, persistantState.apPass.c_str());
}

/**
   @brief:
   Payload Queue element copy the payload object to the respective payload depending on sensor profile
*/
class PayloadQueueElement
{
  public:
    // this will keep a copy of sensor payload, the ptr doens't have to be valid after the call
    PayloadQueueElement(const char *mac, SensorPayload *payload)
    {
      _mac = strdup(mac);
      _payload = copyPayloadObject(payload);
    }
    ~PayloadQueueElement()
    {
      freePayloadObject(_payload);
      _payload = nullptr;
      free(_mac);
      _mac = nullptr;
    }

    char * _mac;
    SensorPayload* _payload;
};


class DeviceState
{
  public:
    // public data members
    RunTimeState        runTimeState;
    PersistantState     persistantState;
    SensorPayloadTH    telemetry;
    LinkedList<PayloadQueueElement*> telemetryQueue;
    DeviceState() {
      /**
         @todo:There was a problem in begining it here
      */
      //EEPROM.begin(EEPROM_STORE_SIZE);
    }
    ~DeviceState() {
      EEPROM.end();
    }

    void enqueSensorPayload(const char *mac, SensorPayload *payload)
    { 
        
      if (!mac || !payload) {
        DEBUG_PRINTLN("invalid data in payload");
        return;
      }

      DEBUG_PRINTLN("Allocating new queue element");
      PayloadQueueElement * telemetryElement = new PayloadQueueElement(mac, payload);
      if (!telemetryElement->_payload) {
        DEBUG_PRINTF("copy of payload had failed, not queuing\n");
        delete telemetryElement;
        return;
      }
      
      telemetryQueue.add(telemetryElement);
    }

    
    /**
       @brief:Checks for unprocessed telementry in linkedlist
    */
    bool hasUnprocessedTelemetry() {
      return ( telemetryQueue.size() != 0 );
    }

    /**
       @brief:Load and Store helper functions
    */
    bool store()
    {
      bool retValue = false;
      retValue = storeEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem Storing to EEPROM");
        return false;
      }
      return retValue;
    }

    bool load()
    {
      bool retValue = false;
      retValue = loadEEPROM();
      if (!retValue) {
        DEBUG_PRINTLN("Problem loading from EEPROM");
        return false;
      }
      return retValue;
    }

  
  private:
    PersistantState eepromRealState;

    bool storeEEPROM()
    {
      if (persistantState == eepromRealState) {
        DEBUG_PRINTLN("nothing to write, state hasn't changed since last read/write");
        return true;
      }

      DEBUG_PRINTLN("Writing EEPROM, in memory structure is dirty");
      PersistantStateStorageFormat persistantStore(persistantState);
      EEPROM.put(0, persistantStore);
      EEPROM.commit();
      eepromRealState = persistantState;
      return true;
    }

    bool loadEEPROM() {
      PersistantStateStorageFormat persistantStore;
      EEPROM.get(0, persistantStore);
      if (strcmp(persistantStore.version, EEPROM_STORAGE_FORMAT_VERSION) != 0) {
        DEBUG_PRINTLN("storage format doens't match, let defaults load, will become proper in next write.");
        return true;
      }
      persistantState = PersistantState(persistantStore);
      eepromRealState = persistantState;
      return true;
    }

    bool storeSPIFF()
    {
      return true;
    }



};

extern DeviceState& deviceState;

// just shortening macros
#define RSTATE   deviceState.runTimeState
#define PSTATE   deviceState.persistantState

#endif // DEVICESTATE_H
