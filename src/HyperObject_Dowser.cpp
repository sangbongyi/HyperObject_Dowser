/* 
  Hyper object dowser project
  Copyright (c) 2024 Sangbong Lee <sangbong@me.com>
  
  * Stepper motor controller that rotate brass rods depends on the signal magnitude from WiFi devices and satellites on the sky
  * This code allows the esp32 board to scan the existence of WiFi routers, and the location of satellites that are tracked based on the current GPS position of the device. 
  * Then, it calculates the number of sources which emit signals and magnitudes of those signal from the sources.
  * Finally, the device converts the number of sources and magnitude of signals to the rotation value of the stepper motor for the dowsing rod.

  This work is licensed under the Creative Commons Attribution 4.0 International License.
  To view a copy of this license, visit http://creativecommons.org/licenses/by/4.0/.
*/

#include <Arduino.h>
#include "HyperObject_Dowser.h"

void setup() {
  // Software Serial Initialize
  Serial2.begin(9600, SERIAL_8N1, 12, 13); //SoftSerial rx 12 <-> gps tx, SoftSerial tx 13 <-> gps rx

  all_module_setup();
  get_module_notifications();
  display_init_page(sat_noti, wifi_noti);
}

void loop() {
  update_time();
  update_gps();
  update_sats();
  update_Wifi();
  update_display();
  update_angle();
  update_angle_display();
  update_stepper();
}

void all_module_setup() {
  display.Display_setup();
  wifi.WiFi_setup();
  sat.GPS_setup();
  sat.Sat_init();
  stepper.setup();
}

void get_module_notifications() {
  wifi_noti.wifiNoti =  wifi.Get_notiWifi();
  wifi_noti.wifiNum = wifi.Get_WifiNum();
  
  sat_noti.gpsDataNoti = sat.Get_notiGpsData();
  sat_noti.gpsLocNoti = sat.Get_notiGpsLoc();
  sat_noti.satNoti = sat.Get_notiSat();
  sat_noti.gpsSatNum = sat.Get_gpsNum();
  sat_noti.actSatNum = sat.Get_satNum();

  time_noti.timeSource = sat.Get_timesource();
  time_noti.epochTime = sat.Get_epochTime();
}

void display_init_page(sat_notifications _satNoti, wifi_notifications _wifiNoti) {
  display.Display_draw_firstPage(_satNoti.gpsDataNoti.c_str(), _satNoti.gpsLocNoti.c_str(), _wifiNoti.wifiNoti.c_str(), _satNoti.satNoti.c_str());
}

void update_time() {
  sat.GPS_time_update();
}

void update_gps() {
  sat.GPS_update();
}

void update_sats() {
  sat.Sat_update();
}

void update_Wifi() {
  wifi.WiFi_update();
}

void update_angle() {
  int rotationAngle;
  float rotateAngleWifi = stepper.Get_wifi_angle(wifi.Get_numWifi(), wifi.Get_numWifi());
  float rotateAngleSat  = stepper.Get_sat_angle(sat.sat_vis, sat.sat_ele, sat.sat_dist);
  
  rotationAngle = rotateAngleWifi + rotateAngleSat;

  angles.wifi_angle = rotateAngleWifi;
  angles.sat_angle = rotateAngleSat;
  angles.rotation_angle = rotationAngle;
}

void update_stepper() {
  stepper.update(angles.rotation_angle);
}

void update_display() {
  get_module_notifications();
  display.Display_update(sat_noti.gpsLocNoti.c_str(), time_noti.timeSource.c_str(), time_noti.epochTime.c_str(), 
                         wifi_noti.wifiNum.c_str(),   sat_noti.gpsSatNum.c_str(),   sat_noti.actSatNum.c_str());
}

void update_angle_display() {
  display.Display_rotation_angle(String(angles.sat_angle).c_str(), String(angles.wifi_angle).c_str(), String(angles.rotation_angle).c_str());
}
