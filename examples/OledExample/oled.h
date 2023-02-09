#ifndef OLED_STATE_H_
#define OLED_STATE_H_

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>
#include <Fonts/FreeSerif12pt7b.h>
#include <Fonts/FreeSerifBold12pt7b.h>
#include <Fonts/FreeSerifBold9pt7b.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>
#include <Fonts/FreeMono9pt7b.h>
#include "utils.h"
#include "hardwareDefs.h"
#include "deviceState.h"

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool initDisplay()
{
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    DEBUG_PRINTLN("SSD1306 allocation failed");
    setBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
    return false;
  }
  clearBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
  return true;
}

bool isDisplayAvailable()
{
  return !testBit(RSTATE.deviceEvents, DSE_DisplayDisconnected);
}

void clearDisplay()
{
  delay(100); //TODO:: don't know why this is needed.
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void drawAnotation(String annotation, int16_t cursorPositionY, int16_t cursorPositionX )
{   
    display.setFont();
    display.setTextSize(1);
    display.setCursor(cursorPositionX, cursorPositionY);
    display.println(annotation);
}

void drawLoGo(){
    display.display();
}

  

#endif