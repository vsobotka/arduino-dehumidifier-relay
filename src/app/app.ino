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

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <dht.h>

#define dht_dpin A0

// set the LCD address to 0x27 for a 16 chars 2 line display
// A FEW use address 0x3F
// Set the pins on the I2C chip used for LCD connections:
//                    addr, en,rw,rs,d4,d5,d6,d7,bl,blpol
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
dht DHT;

// proměnné pro nastavení propojovacích pinů
int pinCLK = 3;
int pinDT  = 4;
int pinSW  = 5;

// proměnné pro uložení pozice a stavů pro určení směru
// a stavu tlačítka
int poziceEnkod = 0;
int stavPred;
int stavCLK;
int stavSW;

volatile int seconds;

bool refresh = false;

String lcdRowHumidity;
String lcdRowTemperature;
String lcdRowHumidityOld;
String lcdRowTemperatureOld;

void setup()   /*----( SETUP: RUNS ONCE )----*/
{
  Serial.begin(9600);  // Used to type in characters

  /* Setup LCD */
  lcd.begin(16,2);
  
  /* Setup rotary encoder */
  pinMode(pinCLK, INPUT);
  pinMode(pinDT, INPUT);
  pinMode(pinSW, INPUT_PULLUP);
  stavPred = digitalRead(pinCLK);
  
  /* Setup timer to 1s */
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

void loop()
{
  stavCLK = digitalRead(pinCLK);
  if (stavCLK != stavPred) {
    if (digitalRead(pinDT) != stavCLK) {
      Serial.print("Rotace vpravo => | ");
      poziceEnkod ++;
    }
    else {
      Serial.print("Rotace vlevo  <= | ");
      poziceEnkod--;
    }
    Serial.print("Pozice enkoderu: ");
    Serial.println(poziceEnkod);
  }
  stavPred = stavCLK;
  stavSW = digitalRead(pinSW);
  if (stavSW == 0) {
    Serial.println("Stisknuto tlacitko enkoderu!");
    delay(500);
  }

  if (refresh) {
      getSensorData();
      updateLCD();

      refresh = false;
  }
}

ISR(TIMER1_COMPA_vect)
{
    seconds++;

    if (seconds % 5 == 0) {
        refresh = true;
    }
    
    if (seconds == 10)
    {
        seconds = 0;
    }
}

void getSensorData() {
  DHT.read11(dht_dpin);
  
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

