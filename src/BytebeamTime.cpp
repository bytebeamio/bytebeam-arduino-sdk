#include "BytebeamTime.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

BytebeamTime::BytebeamTime() {
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

boolean BytebeamTime::begin() {
    // begin the time client
    timeClient.begin();

    // set the gmt time offset ( +5:30h = 19800s )
    timeClient.setTimeOffset(19800);

    // set the ntp server update interval to 30s
    timeClient.setUpdateInterval(30);

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
    // update the time client
    timeClient.update();

    // get the epcoh time from the server
    unsigned long time  = timeClient.getEpochTime();

    // calculate epoch millis
    unsigned long long timeMillis = ((unsigned long long)time * 1000) + (millis() % 1000);

    // save the epoch millis
    this->nowMillis = timeMillis;

    // make sure prev epoch millis shouldn't be greater than the now epoch millis
    if(this->prevMillis > this->nowMillis) {
        Serial.println("failed to obtain time");
        return false;
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
    
    // end the time client
    timeClient.end();

    // reset the time variables except durationMillis i.e we need to pump up the duration
    this->beginMillis = 0;
    this->prevMillis = 0;
    this->nowMillis = 0;
    this->endMillis = 0;

    // log end information to serial :)
    Serial.println("TIME : Client end! ");

    return true;
}