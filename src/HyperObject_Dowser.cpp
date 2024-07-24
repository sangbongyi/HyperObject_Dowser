/* 
  Hidden dependency project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * Frequency generator for 3 speakers connected to a Teensy 4.0 board with 2 Audio-shield rev D
  * This code allows the Teensy board 4.0 to generate certain frequencies for 3 speakers to draw laser patterns on the wall.
  * It also has functions to interact between audiences and the device with a microphone module.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

#include <Arduino.h>

// Libraries
#include <Sgp4.h>
#include <U8g2lib.h>
#include <AccelStepper.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include "time.h"

#define ONE_REVOLUTION 4096

#define motor_1_Pin1  2                 // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_1_Pin2  17                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_1_Pin3  21                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_1_Pin4  22                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define motor_2_Pin1  23                // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_2_Pin2  25                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_2_Pin3  32                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_2_Pin4  33                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define MYLAT 53.1061864 //dummy
#define MYLON 8.8643598
#define MYALT 13
#define MYUNIXTIME 1618431790

#define INTERVAL 3000

// GPS Instance
TinyGPSPlus gps;

// GPS notification string
String gpsDataNoti;
String gpsLocNoti;
String timeSource;

float Flat, Flon, Alt, timeGps;
long timeSinceLastEpochUpdate = 0;

// Define some steppers and the pins the will use
AccelStepper stepper1(AccelStepper::HALF4WIRE, motor_1_Pin1, motor_1_Pin3, motor_1_Pin2, motor_1_Pin4);
AccelStepper stepper2(AccelStepper::HALF4WIRE, motor_2_Pin1, motor_2_Pin3, motor_2_Pin2, motor_2_Pin4);

// Variables for the steppers
float steps_1;
float steps_2;
boolean initStepper = false;

// Display
U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);

// WiFi notification string
String wifiNoti;

//================Wifi configuratoin================//
const char* ssid       = "TP-Link_F3E0";
const char* password   = "45876158";
const char* ntpServer = "pool.ntp.org";

int numWifi = 1;
int magWifi = 0;

int rotateAngleWifi = 0;
int rotateAngleSat = 0;
int rotateAngleLora = 0;

// Satellite
Sgp4 sat_1; // ISS         (Space station)
Sgp4 sat_2; // METOP-B     (Weather)
Sgp4 sat_3; // IRIDIUM 105 (Communication)
Sgp4 sat_4; // GSAT0220    (Navigation)
Sgp4 sat_5; // CASSIOPE    (Scientific)
Sgp4 sat_6; // MTI         (Military)

Sgp4 *satellites[6] = {
  &sat_1, &sat_2, &sat_3, &sat_4, &sat_5, &sat_6
};

// Satellite notification string
String satNoti;
//================Satellite tracking information================//

/*struct Sat_TrackInfo {
  char sat_satname[20];
  char sat_tle_line1[70];
  char sat_tle_line2[70];
};*/

struct Sat_TrackInfo {
  String sat_satname;
  String sat_tle_line1;
  String sat_tle_line2;
};

Sat_TrackInfo satInfo_1;
Sat_TrackInfo satInfo_2;
Sat_TrackInfo satInfo_3;
Sat_TrackInfo satInfo_4;
Sat_TrackInfo satInfo_5;
Sat_TrackInfo satInfo_6;

Sat_TrackInfo *satInfos[6] = {
  &satInfo_1, &satInfo_2, &satInfo_3, &satInfo_4, &satInfo_5, &satInfo_6
};
//================Satellite configuratoin================//

int current_gps_sat_num = 0; //gps sat number
int current_tr_act_sat_num = 0; //tracking sat (active)

int numberSat = 6;

float sat_ele[6];
float sat_dist[6];
float sat_vis[6];

float myLat = 53.1061864;
float myLong = 8.8643598;
float myAlt = 13;

int timezone = 2 ;  //utc + 2
int  year; int mon; int day; int hr; int minute; double sec;

unsigned long epochTime;
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}
//=========================================================//

//======================== Functions ======================//
// WiFi functions
void WiFi_setup();
void WiFi_update();

// GPS functions
void GPS_setup();
void GPS_update();

// Sat functions
void Sat_setup(Sgp4 &_sat, char _satname[], char _tle_line1[], char _tle_line2[]);
void Sat_setup(Sgp4 &_sat, Sat_TrackInfo &_sat_info);
void Sat_update();
void Sat_tick(Sgp4 &_sat);
void Sat_track_init();
void Sat_tracking();

// Stepper functions
void Stepper_setup();
void Stepper_update();

// Display functions
void Display_setup(const char *gps_data, 
                   const char *gps_loc, 
                   const char *lora_init, 
                   const char *wifi_init, 
                  const char *sat_init);
void Display_draw(const char *s);
void Display_update(const char *gps_available, 
                    const char *time_source, 
                    const char *epoch_time, 
                    const char *wifi_num, 
                    const char *lora_packet, 
                    const char *g_sats_num, 
                    const char *t_sats_num, 
                    const char *rot_angle);
void Display_rotation(const char *gps_Angle, 
                      const char *lora_Angle, 
                      const char *wifi_Angle);
void Display_init();

// Get stepper angle functions
void Get_wifi_angle();
void Get_sat_angle();
void Get_rotation_angle();

// Get Time
void Update_time();
int Cal_unix_time();

//=========================================================//

void setup() {
  // put your setup code here, to run once:
  
}

void loop() {
  // put your main code here, to run repeatedly:
}

//======================== Function definitions ======================//
//============================ WiFi functions ========================//

// Wifi setup to get unixtime 
void WiFi_setup() {
  WiFi.mode(WIFI_STA);
  wifiNoti = "WIFI STA";
  configTime(0, 0, ntpServer);
}

// Get the number of WiFi devices nearby
void WiFi_update() {
  magWifi = 0; //signal magnitude reset

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

  if (n != 0) {
    //Serial.print(n);
    //Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      //Serial.print(i + 1);
      //Serial.print(": ");
      //Serial.print(WiFi.SSID(i));
      //Serial.print(" (");
      //Serial.print(WiFi.RSSI(i));
        
      magWifi += 100 + WiFi.RSSI(i);
        
      //Serial.print(")");
      //Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
      delay(10);
    }

    numWifi = n;
    //Serial.println();
    //Serial.println("numWifi = " + String(numWifi));
    //Serial.println("magWifi = " + String(magWifi));
  }
}

//============================ GPS functions ========================//

void GPS_setup() {
  bool newGpsData = false;

  // Check if the module is working
  while (Serial2.available())
  {
    char c = Serial2.read();
    if (gps.encode(c)) { // Did a new valid sentence come in?
      
      gpsDataNoti = "DATA OK";
      
      if(gps.location.isUpdated()) {
        newGpsData = true;
      }
    } 
    else {
      gpsDataNoti = "NO DATA";
    }
  }

  // Initialize GPS data
  if(newGpsData) {
    // Get GPS data from the module
    myLat = gps.location.lat();
    myLong = gps.location.lng();
    myAlt = gps.altitude.meters();
    current_gps_sat_num = gps.satellites.value();
    gpsLocNoti = "LOC OK";
  } else {
    // Get GPS data from the internal data
    myLat = MYLAT;
    myLong = MYLON;
    myAlt = MYALT;
    current_gps_sat_num = 0;
    gpsLocNoti = "NO LOC";
  }
  delay(250);
}

// Update GPS Location
void GPS_update() {
  
  bool newGpsData = false;

  // For one second we parse GPS data and report some key values
  for (unsigned long start = millis(); millis() - start < INTERVAL;) {
    // Check if the module is working
    while (Serial2.available()) {
      char c = Serial2.read();
      if (gps.encode(c)) { // Did a new valid sentence come in?
        if(gps.location.isUpdated()) {
          newGpsData = true;
        }
      }
    }
  }

  // Update GPS data
  if (newGpsData) {
    // Get GPS data from the module
    myLat = gps.location.lat();
    myLong = gps.location.lng();
    myAlt = gps.altitude.meters();
    current_gps_sat_num = gps.satellites.value();
    gpsLocNoti = "OK";
  } else {
    gpsLocNoti = "NO";
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) {
    gpsLocNoti = "CHK";
  }
}

//============================ Satellite functions ========================//

void Sat_setup(Sgp4 &_sat, char _satname[], char _tle_line1[], char _tle_line2[]) {
  // Set site latitude[°], longitude[°] and altitude[m]
  _sat.site(myLat, myLong, myAlt);              
  // Initialize satellite parameters 
  _sat.init(_satname, _tle_line1, _tle_line2);  

  // Display TLE epoch time
  double jdC = _sat.satrec.jdsatepoch;
  invjday(jdC , timezone, true, year, mon, day, hr, minute, sec);
  
  satNoti = "SATs OK";
}

void Sat_setup(Sgp4 &_sat, Sat_TrackInfo &_sat_info) {
  // Set site latitude[°], longitude[°] and altitude[m]
  _sat.site(myLat, myLong, myAlt);              
  // Initialize satellite parameters 
  _sat.init(_sat_info.sat_satname.c_str(), 
            _sat_info.sat_tle_line1.begin(), 
            _sat_info.sat_tle_line2.begin());  

  // Display TLE epoch time
  double jdC = _sat.satrec.jdsatepoch;
  invjday(jdC , timezone, true, year, mon, day, hr, minute, sec);
  
  satNoti = "SATs OK";
}

void Sat_update() {
  // Get values from SAT instances and update
  for (int i = 0; i < 6; i++) {
    sat_ele[i] = satellites[i]->satEl;
    sat_dist[i] = satellites[i]->satDist;
    sat_vis[i] = satellites[i]->satVis;
  }
}

void Sat_tick(Sgp4 &_sat) {
  //reset value
  invjday(_sat.satJd , timezone,true, year, mon, day, hr, minute, sec);
  
  switch(_sat.satVis)
  {
    case -2:
        //Serial.println("Visible : Under horizon");
        break;
    case -1:
        //Serial.println("Visible : Daylight");
        current_tr_act_sat_num += 1;
        break;
    default:
        //Serial.println("Visible : " + String(_sat.satVis));   //0:eclipsed - 1000:visible
        current_tr_act_sat_num += 1;
        break;
  }
}

// Assgin tracking informations for the satellites
void Sat_track_init() {
  //ISS //sat 1
  satInfo_1.sat_satname = "ISS (ZARYA)";
  satInfo_1.sat_tle_line1 = "1 25544U 98067A   21087.71025469  .00002067  00000-0  45739-4 0  9999";  //Line one from the TLE data
  satInfo_1.sat_tle_line2 = "2 25544  51.6464  20.7646 0003055 154.7194 281.0675 15.48958175276127";  //Line two from the TLE data                          

  //METOP-B //sat 2
  satInfo_2.sat_satname = "METOP-B";
  satInfo_2.sat_tle_line1 = "1 38771U 12049A   21088.42660194  .00000013  00000-0  26066-4 0  9991";  //Line one from the TLE data
  satInfo_2.sat_tle_line2 = "2 38771  98.6883 149.5866 0000622 205.5689 248.0574 14.21501566442500";  //Line two from the TLE data

  //IRIDIUM 105 //sat 3
  satInfo_3.sat_satname = "IRIDIUM 105";
  satInfo_3.sat_tle_line1 = "1 41921U 17003E   21088.16153075  .00000096  00000-0  27190-4 0  9993";  //Line one from the TLE data
  satInfo_3.sat_tle_line2 = "2 41921  86.3962 133.3714 0002853  89.5521 270.6002 14.34215979221943";  //Line two from the TLE data

  //GSAT0219 //sat 4
  satInfo_4.sat_satname = "GSAT0219 (PRN E36)";
  satInfo_4.sat_tle_line1 = "1 43566U 18060C   21089.27743343 -.00000070  00000-0  00000-0 0  9990";  //Line one from the TLE data
  satInfo_4.sat_tle_line2 = "2 43566  56.9810  33.2423 0004834 303.2320  56.7006  1.70475397 16716";  //Line two from the TLE data

  //CASSIOPE //sat 5
  satInfo_5.sat_satname = "CASSIOPE";
  satInfo_5.sat_tle_line1 = "1 39265U 13055A   21088.22248306  .00004440  00000-0  12365-3 0  9997";  //Line one from the TLE data
  satInfo_5.sat_tle_line2 = "2 39265  80.9711  16.2739 0651440 189.1543 169.7291 14.31616725388518";  //Line two from the TLE data

  //MTI //sat 6
  satInfo_6.sat_satname = "MTI";
  satInfo_6.sat_tle_line1 = "1 26102U 00014A   21088.46105684  .00010874  00000-0  13803-3 0  9997";  //Line one from the TLE data
  satInfo_6.sat_tle_line2 = "2 26102  97.4728 350.8646 0004790 253.6177 106.4580 15.59602769168877";  //Line two from the TLE data

  // Satellite tracking Initialize
  for (int i = 0; i < 6; i++) {
    Sat_setup(*satellites[i], *satInfos[i]);
  }
}

void Sat_tracking() {
  //findsat
}