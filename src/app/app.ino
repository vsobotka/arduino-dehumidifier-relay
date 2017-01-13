/*
  arduino-dehumidifier-relay
  
  Repository:
  - https://github.com/vsobotka/arduino-dehumidifier-relay

  Controls external dehumidifier with relay based on desired and wanted humidity in a room.

  The circuit:
  * LCD: 16x2 blue with PCF8574AT A0A1A2 driver
  * - http://arduino-info.wikispaces.com/LCD-Blue-I2C#v1
  * - https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads
  * Humidity sensor: DHT11
  * Rotary encoder: KY-040﻿
*/

#include <LiquidCrystal_I2C.h>
#include <dht.h>

#define dhtDataPin A0
dht DHT;

/* Rotary encoder pins */
const int rePinCLK = 3;
const int rePinDT  = 4;
const int rePinSW  = 5;
/* States for rotary encoder */
int reValue = 0;
int reOldValueCLK;

/* set the LCD address to 0x27 for a 16 chars 2 line display
 * A FEW use address 0x3F
 * Set the pins on the I2C chip used for LCD connections:
 *                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
*/
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
bool shouldUpdateLCD = false;
String lcdRowHumidity;
String lcdRowTemperature;
String lcdRowHumidityOld;
String lcdRowTemperatureOld;

/* seconds counter */
volatile int seconds;

void setup() {
  Serial.begin(9600);
  initLCD();
  initRotaryEncoder();
  initseconds();
}

void loop() {
  rotaryEncoderEvaluate();

  if (shouldUpdateLCD) {
      getSensorData();
      updateLCD();

      shouldUpdateLCD = false;
  }
}

ISR(seconds1_COMPA_vect) {
    seconds++;

    if (seconds % 5 == 0) {
        shouldUpdateLCD = true;
    }
    
    if (seconds == 10)
    {
        seconds = 0;
    }
}

void getSensorData() {
  DHT.read11(dhtDataPin);
  
  String captionHumidity = "Vlhkost: ";
  double sensorHum = DHT.humidity;
  String unitHumidity = "%";
  
  String captionTemperature = "Teplota: ";
  double sensorTmp = DHT.temperature;
  String unitTemperature = "C";

  lcdRowHumidity = captionHumidity + sensorHum + unitHumidity;
  lcdRowTemperature = captionTemperature + sensorTmp + unitTemperature;
}

void updateLCD() {
  if ((lcdRowHumidityOld != lcdRowHumidity) ||
     (lcdRowTemperatureOld != lcdRowTemperature)) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(lcdRowHumidity);
    lcd.setCursor(0,1);
    lcd.print(lcdRowTemperature);
    lcdRowHumidityOld = lcdRowHumidity;
    lcdRowTemperatureOld = lcdRowTemperature;
  }
}

void initLCD() {
  lcd.begin(16,2);
}

void initRotaryEncoder() {
  pinMode(rePinCLK, INPUT);
  pinMode(rePinDT, INPUT);
  pinMode(rePinSW, INPUT_PULLUP);
  reOldValueCLK = digitalRead(rePinCLK);
}

void initseconds() {
  cli();
  TCCR1A = 0;
  TCCR1B = 0;

  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS10);
  TCCR1B |= (1 << CS12);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void rotaryEncoderEvaluate() {
  int reValueCLK;
  int reValueSW;
  
  reValueCLK = digitalRead(rePinCLK);
  if (reValueCLK != reOldValueCLK) {
    if (digitalRead(rePinDT) != reValueCLK) {
      rotaryEncoderRightRotation();
    } else {
      rotaryEncoderLeftRotation();
    }

    rotaryEncoderAfterChange();
  }
  reOldValueCLK = reValueCLK;
  reValueSW = digitalRead(rePinSW);
  if (reValueSW == 0) {
    rotaryEncoderButtonClick();
  }
}

void rotaryEncoderLeftRotation() {
  Serial.print("Rotace vlevo  <= | ");
  reValue--;
}

void rotaryEncoderRightRotation() {
  Serial.print("Rotace vpravo => | ");
  reValue++;
}

void rotaryEncoderButtonClick() {
  Serial.println("Stisknuto tlacitko enkoderu!");
  delay(500);
}

void rotaryEncoderAfterChange() {
  Serial.print("Pozice enkoderu: ");
  Serial.println(reValue);
}

