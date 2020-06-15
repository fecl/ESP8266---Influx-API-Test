// ****************************************************************************
// MIT License
// Copyright 2020 Ferdinand Clasquin
// 
// Permission is hereby granted, free of charge, to any person obtaining a 
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the 
// Software is furnished to do so, subject to the following conditions:
//  
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//  
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL 
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
// DEALINGS IN THE SOFTWARE. 
// ****************************************************************************

// ****************************************************************************
// Code platform  : PlaformIO (Home Version 3.1.1, Core 4.3.1)
// MCU Plaform    : Espressif 8266 (Version 2.4.0)
// Arduino        : Arduino Wiring-based Framework
//  
// Source Code    : Version 1.40
// History        :
// Date           : April 22, 2020
//
//          *** ESP8266 NodeMCU V1.0 Module Pins ***
//                  
//                      --------------------                                                BME280 T/H/P
//   ADC0 (TOUT)      -| AD0             D0 |- GPIO 16 (User / Wake)                        -------------
//   Reserved         -| RSV             D1 |- GPIO  5 (Wire (I2C) SCL)                 ==>| SCL         |
//   Reserved         -| RVS             D2 |- GPIO  4 (Wire (I2C) SDA)                 ==>| SDA         |
//   GPIO 10  (SDD3)  -| SD3             D3 |- GPIO  0 (Flash)                             |             |
//   GPIO  9  (SDD2)  -| SD2             D4 |- GPIO  2 (TXD1 / OneWire)                    |             |
//   SPI MOSI (SDD1)  -| SD1            3V3 |- 3.3V                                     ==>| VCC         |
//   SPI CS   (SDCMD) -| CMD            GND |- GND                                      ==>| GND         |
//   SPI MISO (SDD0)  -| SD0             D5 |- GPIO 14 (HSPICLK / SPI SCL)                  -------------
//   SPI SCLK (SDCLK) -| CLK             D6 |- GPIO 12 (HSIQ / SPI MISO) 
//   GND              -| GND             D7 |- GPIO 13 (RXD2 / HSID / SPI MOSI) 
//   3.3V             -| 3V3             D8 |- GPIO 15 (TXD2 / HSPICS / SPI (SS) CS)
//   Enable           -| EN              RX |- GPIO  3 (RXD0) 
//   Reset            -| RST             TX |- GPIO  1 (TXD0)  
//   GND              -| GND            GND |- GND  
//   5.0V             -| 5V             3V3 |- 3.3V    
//                      --------------------                                              
//                                                                                      
//
// ****************************************************************************

// ****************************************************************************
//
//
// ****************************************************************************
#include <Arduino.h>
#define DEBUG_ON
#define Speed 115200
#define Config SERIAL_8N1
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
//
// ****************************************************************************

// ****************************************************************************
// Wire / I2C (2 Wire)
// 
// - SCL is on D1 (GPIO05)
// - SDA is on D2 (GPIO04)
#include <Wire.h>
// 
// ****************************************************************************

// ****************************************************************************
// Serial Peripheral Interface (SPI) 4 Wire
// 
// - MOSI is on D7 (GPIOxx)
// - MISO is on D2 (GPIOxx)
// - SCLK is on D5 (GPIOxx)
// - CS   is on D8 (GPIOxx)
#include <SPI.h>
// 
// ****************************************************************************

// ****************************************************************************
// Bosch Sensor BME280 
// 
#include <BME280I2C.h>
//
BME280I2C bme;
float ELEVATION = 446.0; // Level of Elevation Pfaffenhofen an der Ilm
//
// ****************************************************************************

// ****************************************************************************
//
#include "wifi_auth.h"
#include <ESP8266WiFiMulti.h>
ESP8266WiFiMulti wifiMulti;
// WiFi AP SSID
#define WIFI_SSID wifi_ssid
// WiFi password
#define WIFI_PASSWORD wifi_pwrd
//
// ****************************************************************************

// ****************************************************************************
//
#include "influx_auth.h"
#include <InfluxDbClient.h>
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// For InfluxDB 1 e.g. http://192.168.1.48:8086
#define INFLUXDB_URL influx_url
// Set InfluxDB 1 authentication params
#define INFLUXDB_USER influx_user
#define INFLUXDB_PASSWORD influx_pssw
// InfluxDB v1 database name 
#define INFLUXDB_DB_NAME influx_dbase
// InfluxDB client instance
// InfluxDB client instance for InfluxDB 1.x
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);
// Data point
Point sensor(influx_measurement); // Measurement Name
//
// ****************************************************************************

// ****************************************************************************
//
// 1/1024 ADC bit Value
float adcConst = 0.004297;
int nVoltageRaw = 0;
float fVoltage = 0.0;

// Li-Ion Capacity definitions table
// [Voltage Levels][Capacity Volume]
// Based on Panasonic 18650B Battery
float LithiumionVoltageMatrix[22][2] =
{
    {4.20, 100},
    {4.16, 95},
    {4.12, 90},
    {4.08, 85},
    {4.04, 80},
    {4.00, 75},
    {3.96, 70},
    {3.92, 65},    
    {3.88, 60},
    {3.84, 55},
    {3.80, 50},
    {3.76, 45},
    {3.72, 40},
    {3.68, 35},
    {3.64, 30},
    {3.60, 25},
    {3.56, 20},
    {3.52, 15},
    {3.48, 10},
    {3.44, 5},
    {3.40, 0},
    {  0, 0}
};

int i = 0, percentage = 0;
//
// ****************************************************************************

// ****************************************************************************
//
void timeSync() {
  // Synchronize UTC time with NTP servers
  // Accurate time is necessary for certificate validaton and writing in batches
  configTime(0, 0, "pool.ntp.org", "time.nis.gov");
  // Set timezone
  setenv("TZ", TZ_INFO, 1);

  // Wait till time is synced
  #ifdef DEBUG_ON
    Serial.print("Syncing time");
  #endif

  int i = 0;
  while (time(nullptr) < 1000000000ul && i < 100) {
    #ifdef DEBUG_ON
      Serial.print(".");
    #endif
    delay(100);
    i++;
  }
  #ifdef DEBUG_ON
    Serial.println();
  #endif

  // Show time
  time_t tnow = time(nullptr);
  #ifdef DEBUG_ON
    Serial.print("Synchronized time: ");
    Serial.println(String(ctime(&tnow)));
  #endif
}
//
// ****************************************************************************

double SaturationVaporPressure(double Temperature)
{
  return 6.1078 * pow(10.0, ((7.5 * Temperature) / (237.3 + Temperature)));
}

double VaporPressure(double Humidity, double Temperature)
{
  return Humidity / 100 * SaturationVaporPressure(Temperature);
}

double DewpointTemperature(double Humidity, double Temperature)
{
  double v = log10(VaporPressure(Humidity, Temperature) / 6.1078);
  return 237.3 * v / (7.5 - v);
}

double HeatIndex(double Humidity, double Temperature)
{
  double c1 = -8.78469475556;
  double c2 = 1.61139411;
  double c3 = 2.33854883889;
  double c4 = -0.14611605;
  double c5 = -0.012308094;
  double c6 = -0.0164248277778;
  double c7 = 0.002211732;
  double c8 = 0.00072546;
  double c9 = -0.000003582;

  return c1 + (c2 * Temperature) + (c3 * Humidity) + (c4 * Temperature * Humidity) + (c5 * pow(Temperature,2)) + (c6 * pow(Humidity,2)) + (c7 * pow(Temperature,2) * Humidity) + (c8 * Temperature * pow(Humidity,2)) + (c9 * pow(Temperature,2) * pow(Humidity,2));

}


// ****************************************************************************
//
void serial_debug()
{
  // **************************************************************************
  // * Serial Monitor Setup
  // * Serial.begin(speed, config)
  // * Speed  : 2400 - 115200 Baud 
  // * Config : SERIAL_8N1 (default) = 28
  // *
  #ifdef DEBUG_ON  
    Serial.begin(Speed, Config);
    delay(50);
    Serial.println("... Serial Debug ... ");
    Serial.print("Baud rate: ");
    Serial.print(Speed);
    Serial.print(" Line: ");
    Serial.print(Config);
    Serial.println(" ");

    while (!Serial) 
    { 
      ; // wait for serial port to connect.
    }
  #endif
  // *
  // **************************************************************************
}
//
// ****************************************************************************

// ****************************************************************************
//
void setup() {
  // put your setup code here, to run once:
  serial_debug();

  WiFi.disconnect();
  delay(10);

  WiFi.hostname("NodeMCU01_BME280");

  // Connect WiFi
  #ifdef DEBUG_ON 
    Serial.println("Connecting to WiFi");
  #endif

  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  Wire.begin();

  while(!bme.begin())
  {
    #ifdef DEBUG_ON 
      Serial.println("Could not find BME280 sensor!");
    #endif

    delay(1000);
  }

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
      #ifdef DEBUG_ON 
        Serial.println("Found BME280 sensor! Success.");
      #endif
       break;
     case BME280::ChipModel_BMP280:
      #ifdef DEBUG_ON 
        Serial.println("Found BMP280 sensor! No Humidity available.");
      #endif
       break;
     default:
      #ifdef DEBUG_ON 
        Serial.println("Found UNKNOWN sensor! Error!");
      #endif
       break;
  }

  // ************************************************************************
  // Put the Analog Input Port as INPUT
  // ************************************************************************
  pinMode(A0, INPUT);

  // Set InfluxDB 1 authentication params
  client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);

  // Add constant tags - only once
  sensor.addTag("Device", influx_tagkey1);
  sensor.addTag("Sensor", influx_tagkey2);

  // Sync time for certificate validation
  timeSync();

  // Check server connection
  if (client.validateConnection()) {
    #ifdef DEBUG_ON
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    #endif
  } else {
      #ifdef DEBUG_ON
        Serial.print("InfluxDB connection failed: ");
        Serial.println(client.getLastErrorMessage());
      #endif
  }  
}
//
// ****************************************************************************

// ****************************************************************************
//
void loop() {
  // put your main code here, to run repeatedly:

  float temp(NAN), hum(NAN), pres(NAN), dewpoint(NAN), heatidx(NAN);
  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);

  bme.read(pres, temp, hum, tempUnit, presUnit);
  pres = (((pres)/pow((1-((float)(ELEVATION))/44330), 5.255))/100.0);
  dewpoint = DewpointTemperature(hum, temp);
  heatidx = HeatIndex(hum, temp);

  percentage = 0;

  // Take 10 Samples of the ADC A0 and Average the Value for stable ADC
  for (i=0;i<10;i++)
  {
      nVoltageRaw = nVoltageRaw + analogRead(A0);
      delay(20);
  }

  // Calculate the average of the 10 Values
  nVoltageRaw = (float)nVoltageRaw / 10.0;
  Serial.print("ADC RAW Value: ");
  Serial.println(nVoltageRaw);

  // Convert to Voltage Level
  fVoltage = nVoltageRaw * adcConst;
  Serial.print("ADC Voltage Value: ");
  Serial.println(fVoltage);

  // Lookup the Capacity Value
  for(i = 0; LithiumionVoltageMatrix[i][0] > 0; i++) 
  {
      if(fVoltage >= LithiumionVoltageMatrix[i][0]) 
      {
          percentage = LithiumionVoltageMatrix[i][1];
          break;
      }
  }
  Serial.print("Battery Capacity Percentage: ");
  Serial.println(percentage);

  // Store measured value into point
  sensor.clearFields();
  // Report Temperature, Humidity and Air Pressure
  sensor.addField(influx_fieldkey1, temp);
  sensor.addField(influx_fieldkey2, dewpoint);
  sensor.addField(influx_fieldkey3, heatidx);  
  sensor.addField(influx_fieldkey4, hum);
  sensor.addField(influx_fieldkey5, pres);
  sensor.addField(influx_fieldkey6, nVoltageRaw);
  sensor.addField(influx_fieldkey7, fVoltage);
  sensor.addField(influx_fieldkey8, percentage);   

  // Print what are we exactly writing
  #ifdef DEBUG_ON
    Serial.print("Writing: ");
    Serial.println(sensor.toLineProtocol());
  #endif
  // If no Wifi signal, try to reconnect it
  if ((WiFi.RSSI() == 0) && (wifiMulti.run() != WL_CONNECTED))
    #ifdef DEBUG_ON
      Serial.println("Wifi connection lost");
    #endif
  // Write point
  if (!client.writePoint(sensor)) {
    #ifdef DEBUG_ON
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    #endif
  }

  // To put the ESP8266 in deep sleep, you use ESP.deepsleep(uS) and pass as argument the sleep time in microseconds.
  // In this case, 300e6 corresponds to 300.000.000 microseconds which is equal to 300 seconds.
  Serial.println("Going in Deep Sleep for 5min");
  ESP.deepSleep(300e6); 
  
}
//
// ****************************************************************************