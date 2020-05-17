#include <Chrono.h>
#include <LightChrono.h>
#include "MAX31856.h"
#include "Time.h"
#include "TimeLib.h"
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>    // Core graphics library
//#include <Waveshare4InchTftShield.h>
#include <MCUFRIEND_kbv.h>


#include <SPI.h>
#include <stdio.h> // for function sprintf
#include <U8g2_for_Adafruit_GFX.h>


#define SDO    32
#define CS0    34
#define CS1    36
#define CS2    38
#define CS3    40
#define SDI    42
#define SCK    44


#define  pinKilnOff 53


#define CR0_INIT  (CR0_AUTOMATIC_CONVERSION + CR0_OPEN_CIRCUIT_FAULT_TYPE_K /* + CR0_NOISE_FILTER_50HZ */)
#define CR1_INIT  (CR1_AVERAGE_2_SAMPLES + CR1_THERMOCOUPLE_TYPE_K)
#define MASK_INIT (~(MASK_VOLTAGE_UNDER_OVER_FAULT + MASK_THERMOCOUPLE_OPEN_FAULT))
#define NUM_MAX31856   2
// Create the temperature object, defining the pins used for communication
MAX31856 *TemperatureSensor[NUM_MAX31856] = {
  new MAX31856(SDI, SDO, CS0, SCK),
  new MAX31856(SDI, SDO, CS1, SCK)
//  new MAX31856(SDI, SDO, CS2, SCK)
//  new MAX31856(SDI, SDO, CS3, SCK)
};

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

MCUFRIEND_kbv tft;

U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

unsigned int TempL;
unsigned int TempH;
unsigned int TempMax;
unsigned int TempAvg;
unsigned int RatePerHour;

Chrono KilnTime(Chrono::SECONDS);
Chrono CoolTime(Chrono::SECONDS);


uint16_t RowH;

#define DEBUG true

uint32_t OnFlashColor;

void setup() {
 
  for (int i = 0; i < NUM_MAX31856; i++) {
    //Serial.println(i);
    TemperatureSensor[i]->writeRegister(REGISTER_CR0, CR0_INIT);
    TemperatureSensor[i]->writeRegister(REGISTER_CR1, CR1_INIT);
    TemperatureSensor[i]->writeRegister(REGISTER_MASK, MASK_INIT);
  }

  KilnTime.restart();
  KilnTime.stop();
  CoolTime.stop();
  
  Serial.begin(9600);
 
  SPI.begin();
//  Waveshield.begin();
  //tft.Init_LCD();
    static uint16_t g_identifier;
       Serial.print("ID = 0x");
    Serial.println(g_identifier, HEX);
#
    tft.reset();                 //we can't read ID on 9341 until begin()
    g_identifier = tft.readID();
  tft.begin(g_identifier);
  u8g2_for_adafruit_gfx.begin(tft);
 
  u8g2_for_adafruit_gfx.setFont(u8g2_font_inb38_mf);  // select u8g2 font from here: https://github.com/olikraus/u8g2/wiki/fntlistall
  u8g2_for_adafruit_gfx.setFontMode(0);                 // use u8g2 transparent mode (this is default)
  u8g2_for_adafruit_gfx.setFontDirection(1);            // left to right (this is default)
  u8g2_for_adafruit_gfx.setForegroundColor(WHITE);      // apply Adafruit GFX color
  u8g2_for_adafruit_gfx.setBackgroundColor(BLACK);      // apply Adafruit GFX color

 
  
  tft.setRotation(3);
  tft.fillScreen(BLACK);
  u8g2_for_adafruit_gfx.setFont(u8g2_font_inb49_mf);
  RowH = 70;

  
  pinMode(pinKilnOff, INPUT_PULLUP); 
 
    
}

void loop() {

  TempH = TemperatureSensor[0]->readThermocouple(FAHRENHEIT);
  TempL = TemperatureSensor[1]->readThermocouple(FAHRENHEIT);
  TempAvg = 0;

 unsigned int TempAvgCount=0;
  if (TempL > 3000) {TempL = 0;}
  else {TempAvg = TempAvg + TempL;
  TempAvgCount = TempAvgCount + 1;
}
  if (TempH > 3000){TempH = 0;
      }
  else {
  TempAvgCount = TempAvgCount + 1; 
    TempAvg = TempAvg + TempH;
  }
  if (TempAvgCount > 0){
     TempAvg= TempAvg/TempAvgCount;
  }
  if (TempL > TempMax){TempMax = TempL;}
  if (TempH > TempMax){TempMax = TempH;}



char tbs[6];  
if (KilnTime.isRunning() == true)
  {
  if  (digitalRead(pinKilnOff) == HIGH )
  {
          KilnTime.stop();
          CoolTime.restart(0);
 
          u8g2_for_adafruit_gfx.drawStr((RowH * 2) + 5,0,"Lastrun:");
          Serial.print ("OFF");
          
  }
  else
  {
        
          if ( OnFlashColor== RED) {
                OnFlashColor = WHITE;
                u8g2_for_adafruit_gfx.setForegroundColor(OnFlashColor);
                u8g2_for_adafruit_gfx.drawStr((RowH * 2) + 5,0,"*ON*");
          }
          else
          {
              OnFlashColor = RED;
              u8g2_for_adafruit_gfx.setForegroundColor(OnFlashColor);
              u8g2_for_adafruit_gfx.drawStr((RowH * 2) + 5,0,"*ON*");
              u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
                
          }
          
  
  } 
}
else
{
  if  (digitalRead(pinKilnOff) == LOW  )
  {
          KilnTime.restart(0); 
          TempMax = 0; 
          OnFlashColor= RED;
          u8g2_for_adafruit_gfx.setForegroundColor(OnFlashColor);      // apply Adafruit GFX color  
          u8g2_for_adafruit_gfx.drawStr((RowH *2)+5,0,"*ON*     ");
          u8g2_for_adafruit_gfx.setForegroundColor(WHITE);      // apply Adafruit GFX color
  }
}
if (  CoolTime.isRunning() == true )
{
     if (TempH < 400){ 
        CoolTime.stop();
     }

}

u8g2_for_adafruit_gfx.setFont(u8g2_font_inb49_mf);
sprintf(tbs, "%4df", TempH); u8g2_for_adafruit_gfx.drawStr(480 - (RowH *1),0,tbs);
sprintf(tbs, "%4df", TempL); u8g2_for_adafruit_gfx.drawStr(480 - (RowH *2),0,tbs);
sprintf(tbs, "%4dM", TempMax); u8g2_for_adafruit_gfx.drawStr(480 - (RowH *3),0,tbs);

//run time
int Seconds = (int)KilnTime.elapsed() % 60; int Minutes = (int)(KilnTime.elapsed() / 60); int Hours = (int)(Minutes / 60);
char tbs2[10];  
sprintf(tbs2, "H%d:%02d:%02d", Hours,Minutes,Seconds); u8g2_for_adafruit_gfx.drawStr(5+ RowH,0,tbs2);

//cool time
 Seconds = (int)CoolTime.elapsed() % 60;  Minutes = (int)(CoolTime.elapsed() / 60);  Hours = (int)(Minutes / 60);
  
sprintf(tbs2, "C%d:%02d:%02d", Hours,Minutes,Seconds); u8g2_for_adafruit_gfx.drawStr(5,0,tbs2);


delay(1000);
}
