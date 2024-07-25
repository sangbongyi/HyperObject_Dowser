/* 
  Hyper object dowser project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * Stepper motor controller that rotate brass rods depends on the signal magnitude from WiFi devices and satellites on the sky
  * This code allows the esp32 board to scan the existence of WiFi routers, and the location of satellites that are tracked based on the current position of the device. 
  * Then, it calculates the number of sources which emit signals and magnitudes of those signal from the sources.
  * Finally, the device converts the number of sources and magnitude of signals to the rotation value of the stepper motor for the dowsing rod.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

#include <Arduino.h>
#include "HyperObject_Dowser.h"

//=========================================================//

void setup() {
  // Software Serial Initialize
  Serial2.begin(9600, SERIAL_8N1, 12, 13); //SoftSerial rx 12 <-> gps tx, SoftSerial tx 13 <-> gps rx

  // Display Initialize
  Display_setup();

  // Stepper motor Initialize
  Stepper_setup();
  
  // WiFi Initialize
  WiFi_setup();
  
  // GPS Initialize
  GPS_setup();
  
  // Satellite tracking Initialize
  Sat_track_init();

  // Draw first page of data from modules
  Display_draw_firstPage(gpsDataNoti.c_str(), gpsLocNoti.c_str(), wifiNoti.c_str(), satNoti.c_str());
}

void loop() {
  Update_time();

  GPS_update();

  Sat_tracking();

  //Display SATs infos => Sat names, Azimuth, Latitude, Longitude, distance,......
  current_tr_act_sat_num = 0;
  for (int i = 0; i < 6; i++) {
    Sat_tick(*satellites[i]);
  }

  Sat_update();

  WiFi_update();
  
  // Calculating rotation angle
  rotateAngleWifi = Get_wifi_angle(numWifi, magWifi);
  rotateAngleSat  = Get_sat_angle();
  int rotationAngle = Get_rotation_angle(rotateAngleWifi, rotateAngleSat);

  // Rotating stepper motors
  Stepper_update(rotationAngle);
  // end Rotating stepper motors

  // Display data 
  Display_update(String(gpsLocNoti).c_str(), String(timeSource).c_str(), String(epochTime).c_str(), 
                    String(numWifi).c_str(), String(current_gps_sat_num).c_str(), 
                    String(current_tr_act_sat_num).c_str(), String(rotationAngle).c_str()); 
  
  // Display rotation angle data 
  Display_rotation_angle(String(rotateAngleSat).c_str(), String(rotateAngleWifi).c_str());


  delay(INTERVAL); // Reduced interval??
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

// Satellite tracking
void Sat_tracking() {
  // findsat
  for(int i = 0; i < 6; i++) {
    satellites[i]->findsat(epochTime);
  }
}

//============================ Display functions ========================//
// Setup display //
void Display_setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.clearDisplay();
  u8g2.setDisplayRotation(U8G2_R1);
  u8g2.setFlipMode(0);
}

// Display initial data from modules //
void Display_draw_firstPage(const char *gps_data, const char *gps_loc, const char *wifi_init, const char *sat_init) {
  u8g2.firstPage();
  do {
    u8g2.drawStr(2,15, "SYS START...");    
    u8g2.drawStr(2,30, gps_data); 
    u8g2.drawStr(2,45, gps_loc);    
    u8g2.drawStr(2,60, wifi_init);
    u8g2.drawStr(2,75, sat_init);
    
  } while (u8g2.nextPage());
  delay(2500);
}

void Display_draw(const char *s) {
  u8g2.firstPage();
  do {  
    u8g2.drawStr(2,15,s); 

  } while (u8g2.nextPage());
  delay(500);
}

void Display_update(const char *gps_available, const char *time_source, const char *epoch_time, const char *wifi_num, 
                    const char *g_sats_num, const char *t_sats_num, const char *rot_angle) {
  u8g2.clearDisplay();
  u8g2.firstPage();
  do {
    u8g2.drawStr(2,15, "Update...");    
    u8g2.drawStr(2,30, "G_LOC ");
    u8g2.drawStr(40,30, gps_available); 

    u8g2.drawStr(2,45, "TIME ");
    u8g2.drawStr(30,45, time_source);
    
    u8g2.drawStr(2,60, epoch_time);    
    
    u8g2.drawStr(2,75, "WiFi ");
    u8g2.drawStr(30,75, wifi_num);
    
    u8g2.drawStr(2,90, "GS ");
    u8g2.drawStr(18,90, g_sats_num);

    u8g2.drawStr(30,105, "TS ");
    u8g2.drawStr(48,105, t_sats_num);
    
    u8g2.drawStr(2,115, "ANGLE ");
    u8g2.drawStr(35,115, rot_angle);
    
  } while ( u8g2.nextPage() );
  delay(3000);
}

void Display_rotation_angle(const char *gps_Angle, const char *wifi_Angle) {
  u8g2.clearDisplay();
  u8g2.firstPage();
  do {
    u8g2.drawStr(2,15, "Rotate...");    
    
    u8g2.drawStr(2,30, "GPS ");
    u8g2.drawStr(30,30, gps_Angle); 
    
    u8g2.drawStr(2,45, "WIFI "); 
    u8g2.drawStr(30,45, wifi_Angle);    
    
  } while ( u8g2.nextPage() );
}

//============================ Stepper functions ========================//
// Setup stepper movements //
void Stepper_setup() {
  stepper1.setMaxSpeed(500.0);
  stepper1.setCurrentPosition(0);
  stepper1.setAcceleration(150.0);
  stepper1.setSpeed(30);
  
  stepper2.setMaxSpeed(500.0);
  stepper2.setCurrentPosition(0);
  stepper2.setAcceleration(150.0);
  stepper2.setSpeed(30);
}

// Update stepper movements //
void Stepper_update(int _dstAngle) {
  stepper1.moveTo(Get_angle_to_step(_dstAngle));
  stepper2.moveTo(Get_angle_to_step(_dstAngle));

  while(stepper1.distanceToGo() > 0){
    stepper1.run();
    stepper2.run();
  }
  //Serial.println("Rotate reverse " + String(_dstAngle) + " to " + String(0));
  stepper1.moveTo(Get_angle_to_step(_dstAngle*-1));
  stepper2.moveTo(Get_angle_to_step(_dstAngle*-1));

  while(stepper1.distanceToGo() < 0) {
    stepper1.run();
    stepper2.run();
  }

  stepper1.moveTo(0);
  stepper2.moveTo(0);

  while(stepper1.distanceToGo() > 0) {
    stepper1.run();
    stepper2.run();
  }
  
  //Elapsed time check?
  
  //Serial.println("Rotation complete..!!");
  previousAngle = _dstAngle; 
  //Serial.println();
}

//============================ Get stepper angle functions ========================//
int Get_wifi_angle(int _num, int _mag) {
  int angle = 0;
  angle = _num + abs(_mag / _mag);
  angle = constrain(angle, 0, 15);

  return angle;
}
int Get_sat_angle() {
  float eleAngle;
  float distAngle;
  float visAngle;
  
  int angle = 0;

  for (int i = 0; i < numberSat; i++) {
    if(sat_vis[i] == -2) 
    { 
      //Under horizon 
      visAngle += (sat_vis[i] * 10); // -20
      eleAngle += 0;
      distAngle += 0;
    } 
    else if(sat_vis[i] == -1)
    {
      //Day light
      visAngle += (sat_vis[i] * 10); // -10
      eleAngle += (180 + sat_ele[i] / 100 * numberSat);  // Positive number  180 + (-180 - 180)
      distAngle += (35000 - sat_dist[i]  / 35000 * numberSat);
    }
    else
    {
      //Visible
      visAngle += (sat_vis[i] / 100); // 0 - 1000
      eleAngle += (180 + sat_ele[i] / 100 * numberSat);  // Positive number  180 + (-180 - 180)
      distAngle += (35000 - sat_dist[i]  / 35000 * numberSat);
    }
  }
  angle = eleAngle + distAngle + visAngle;
  angle = constrain(angle, 0, 60);
  
  return angle;
}
int Get_rotation_angle(int _angleWifi, int _angleSat) {
  int rotation;
  rotation = _angleWifi + _angleSat;

  return rotation;
}
float Get_angle_to_step(float _angle) {
  float stepperStep;
  stepperStep = round(_angle * ONE_REVOLUTION / 360);
  
  return stepperStep;
}

//============================ Calculate time functions ========================//
void Update_time() {
timeSinceLastEpochUpdate = millis();
  if (gps.time.isValid())
  {
    epochTime = Cal_unix_time();
    
    if (epochTime < MYUNIXTIME) {
      epochTime = MYUNIXTIME + timeSinceLastEpochUpdate;
      timeSource = "INTERNAL";
    } else {
      timeSource = "GPS";
    }
    
  } else {
    epochTime = MYUNIXTIME + timeSinceLastEpochUpdate;
    timeSource = "INTERNAL";
  }
}

int Cal_unix_time() {
  time_t t_of_day;
  struct tm t;
  
  t.tm_year = gps.date.year() - 1900;
  t.tm_mon = gps.date.month() - 1;           // Month, 0 - jan
  t.tm_mday = gps.date.day();                 // Day of the month
  t.tm_hour = gps.time.hour();
  t.tm_min =  gps.time.minute();
  t.tm_sec = gps.time.second();
  t_of_day = mktime(&t);

  return t_of_day;
}