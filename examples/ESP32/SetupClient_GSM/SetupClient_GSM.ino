#define TINY_GSM_MODEM_SIM7600
#include <TinyGsmClient.h>
#include <BytebeamArduino.h>

//
// By including the above header you got the access to the gloabl object named Bytebeam
// Use Bytebeam global object to perform the required operations
//

#define LED_PIN 12
#define MODEM_PWRKEY  4
#define MODEM_FLIGHT  25
#define MODEM_TX  27
#define MODEM_RX  26

// set GSM PIN, if any
#define GSM_PIN ""

#define SerialMon Serial
#define SerialAT Serial1

// Your GPRS credentials, if any
const char apn[]      = "";
const char gprsUser[] = "";
const char gprsPass[] = "";

TinyGsm modem(SerialAT);

// function to setup the modem with predefined credentials
void setupModem() {
  // configure your reset, enable pins here if needed
  pinMode(LED_PIN, OUTPUT);
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_FLIGHT, OUTPUT);

  // set the status led pin low to indicate the start process
  digitalWrite(LED_PIN, LOW);

  // pull up power pin (IO:4) Modulator power key, need to powered up the modem
  // this pin must be held high for more than 1 second according to manual requirements
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(100);
  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, LOW);  
  
  // pull up the power pin (IO:25) Modulator flight mode control, need to enable modulator 
  // this pin must be set to high
  digitalWrite(MODEM_FLIGHT, HIGH);

  SerialMon.println("Modem Powered ON");

  // start the serial communication b/w esp32 and modem
  SerialAT.begin(115200, SERIAL_8N1, MODEM_RX, MODEM_TX);
  delay(6000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  modem.restart();
  // modem.init();

  // set the network mode (2 : Automatic)
  modem.setNetworkMode(2);

  String modemName = modem.getModemName();
  SerialMon.printf("Modem Name : ");
  SerialMon.println(modemName);

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem Info : ");
  SerialMon.println(modemInfo);

  // Unlock your SIM card with a PIN if needed
  if(GSM_PIN && modem.getSimStatus() != 3) { 
    modem.simUnlock(GSM_PIN); 
  }

  SimStatus simStatus = modem.getSimStatus();

  if(simStatus != SIM_READY) {
    SerialMon.println("Couldn't Ready the SIM.");
    return;
  }
  SerialMon.println("SIM is Ready.");

  // high indicates the modem is initialized successfully
  digitalWrite(LED_PIN, HIGH);
  SerialMon.println("Modem Initialized Successfully !");

  SerialMon.println("Waiting for network...");
  if(!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if(modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  // GPRS connection parameters are usually set after network registration
  SerialMon.print("Connecting to apn : ");
  SerialMon.println(apn);
  
  if (!modem.gprsConnect(apn)) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");

  if(modem.isGprsConnected()) {
    SerialMon.println("GPRS connected");
  }
}

// function to sync time from ntp server with predefined credentials
void syncTimeFromNtp() {
  String dateTime = modem.getGSMDateTime(DATE_FULL);

  SerialMon.print("Current Time : ");
  SerialMon.println(dateTime);
  SerialMon.println();
}

void setup() {
  // put your setup code here, to run once:
  SerialMon.begin(115200);
  SerialMon.println();

  setupModem();
  syncTimeFromNtp();

  //
  //  Your other application setup stuff goes here
  //

  // This method will initialize and start the bytebeam client
  // You can over-ride the default file system, file name and library logging options, if needed
  // eg. Bytebeam.begin(BytebeamArduino::LITTLEFS_FILE_SYSTEM, "/my_device_config.json", BytebeamLogger::LOG_INFO)
  if(!Bytebeam.begin(&modem)) {
    Serial.println("Bytebeam Client Initialization Failed.");
  } else {
    Serial.println("Bytebeam Client is Initialized Successfully.");
  }

  //
  // If above call is successfull then the bytebeam client is now configured for the use
  // You can always check for the logs in serial monitor for the status of the above call
  //

  // check if bytebeam client is connected or disconnected
  bool connectionStatus = Bytebeam.isConnected();

  if(!connectionStatus) {
    SerialMon.println("Bytebeam Client is Disconnected.");
  } else {
    SerialMon.println("Bytebeam Client is Connected.");
  }

  // Call the end method to stop and de-initialize the bytebeam client at any point of time in the code
  // Bytebeam.end();
}

void loop() {
  // put your main code here, to run repeatedly:

  //
  //  Your application regular stuff goes here
  //

  // This method will let you maintain the connection with the bytebeam cloud, In case
  // the connection is lost, it will attempt to reconnect to the bytebeam cloud
  Bytebeam.loop();

  // software delay, you can customize it as per your application needs
  delay(5000);
}