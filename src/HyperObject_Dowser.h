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

//================Stepper angle variables================//
int rotateAngleWifi = 0;
int rotateAngleSat = 0;
int rotateAngleLora = 0;
int previousAngle = 0;

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

// Display functions
void Display_setup();
void Display_draw_firstPage(const char *gps_data, const char *gps_loc, 
                            const char *wifi_init, const char *sat_init);
void Display_draw(const char *s);
void Display_update(const char *gps_available, const char *time_source, const char *epoch_time, 
                    const char *wifi_num, const char *g_sats_num, const char *t_sats_num, 
                    const char *rot_angle);
void Display_rotation_angle(const char *gps_Angle, const char *wifi_Angle);

// Stepper functions
void Stepper_setup();
void Stepper_update(int _dstAngle);

// Get stepper angle functions
int Get_wifi_angle(int _num, int _mag);
int Get_sat_angle();
int Get_rotation_angle(int _angleWifi, int _angleSat);
float Get_angle_to_step(float _angle);

// Get Time
void Update_time();
int Cal_unix_time();