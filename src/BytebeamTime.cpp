#include "BytebeamTime.h"

BytebeamTime::BytebeamTime()
    #ifdef BYTEBEAM_ARDUINO_USE_WIFI
        : timeClient(udpClient)
    #endif
{
    //
    // Initailizing all the variables with default values here
    //

    this->beginMillis = 0;
    this->prevMillis = 0;
    this->nowMillis = 0;
    this->endMillis = 0;
    this->durationMillis = 0;
}

BytebeamTime::~BytebeamTime() {
    //
    // Nothing much to do here, just print the log :)
    //

    Serial.println("I am BytebeamTime::~BytebeamTime()");
}

#ifdef BYTEBEAM_ARDUINO_USE_MODEM
    void BytebeamTime::setModemInstance(TinyGsm* modem) {
        this->modem = modem;
    }
#endif

boolean BytebeamTime::begin() {
#ifdef BYTEBEAM_ARDUINO_USE_WIFI
    // begin the time client
    timeClient.begin();

    // set the gmt time offset ( +5:30h = 19800s )
    // fix : commenting as of now as we are getting the correct time stamp
    // timeClient.setTimeOffset(19800);

    // set the ntp server update interval to 30s
    timeClient.setUpdateInterval(30);
#endif

    // get the epoch millis
    if(!getEpochMillis()) {
        Serial.println("failed to get epoch millis");
        return false;
    }

    // save the begin epoch millis
    this->beginMillis = this->nowMillis;

    // reset the durationMillis i.e we will start a new session
    this->durationMillis = 0;

    // log begin information to serial :)
    Serial.println("TIME : Client begin! ");

    return true;
}

boolean BytebeamTime::getEpochMillis() {
#ifdef BYTEBEAM_ARDUINO_USE_WIFI
    // update the time client
    timeClient.update();

    // get the epcoh time from the server
    unsigned long time  = timeClient.getEpochTime();
#endif

#ifdef BYTEBEAM_ARDUINO_USE_MODEM
    // will give the time in this format "YY/MM/DD,HH:MM:SS"
    String dateTime = this->modem->getGSMDateTime(DATE_FULL);

    struct tm tm;
    memset(&tm, 0, sizeof(tm));

    tm.tm_year = 2000 + dateTime.substring(0,2).toInt() - 1900;
    tm.tm_mon  = dateTime.substring(3,5).toInt() - 1;
    tm.tm_mday = dateTime.substring(6,8).toInt();

    // Serial.println(tm.tm_year);
    // Serial.println(tm.tm_mon);
    // Serial.println(tm.tm_mday);

    tm.tm_hour = dateTime.substring(9,11).toInt();
    tm.tm_min  = dateTime.substring(12,14).toInt();
    tm.tm_sec  = dateTime.substring(15,17).toInt();

    // Serial.println(tm.tm_hour);
    // Serial.println(tm.tm_min);
    // Serial.println(tm.tm_sec);

    time_t time;
    time = mktime(&tm) - 19800;   // GMT +5:30h
#endif

    // calculate epoch millis
    unsigned long long timeMillis = ((unsigned long long)time * 1000) + (millis() % 1000);

    // save the epoch millis
    this->nowMillis = timeMillis;

    // Serial.println(prevMillis);
    // Serial.println(nowMillis);

    unsigned long long threshold = 1000;

    // make sure prev epoch millis shouldn't be greater than the now epoch millis
    if(this->prevMillis > this->nowMillis) {
        // fix : taking threshold to over come the override that happens when new millis cross high in MSB
        if(this->prevMillis > this->nowMillis + threshold) {
            Serial.println("failed to obtain time");
            return false;
        } else {
            this->nowMillis += threshold;
            // Serial.println("threshold added to epoch millis");
        }
    }

    // update the prev epoch millis
    this->prevMillis = this->nowMillis;

    return true;
}

boolean BytebeamTime::end() {
    // get the epoch millis
    if(!getEpochMillis()) {
        Serial.println("failed to get epoch millis");
        return false;
    }

    // save the end epoch millis
    this->endMillis = this->nowMillis;

    // calculate the duration epoch millis
    this->durationMillis = this->endMillis - this->beginMillis;
    
#ifdef BYTEBEAM_ARDUINO_USE_WIFI
    // end the time client
    timeClient.end();
#endif

    // reset the time variables except durationMillis i.e we need to pump up the duration
    this->beginMillis = 0;
    this->prevMillis = 0;
    this->nowMillis = 0;
    this->endMillis = 0;

    // log end information to serial :)
    Serial.println("TIME : Client end! ");

    return true;
}