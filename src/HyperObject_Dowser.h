//#include <Arduino.h>
#include "HyperObject_Display.h"
#include "HyperObject_Satellite.h"
#include "HyperObject_WiFi.h"
#include "HyperObject_Stepper.h"

Stepper stepper;
HO_Display display;
HO_WiFi wifi;
Sat sat;

struct sat_notifications {
    String gpsDataNoti;
    String gpsLocNoti;
    String satNoti;
    String gpsSatNum;
    String actSatNum;
};

struct wifi_notifications {
    String wifiNoti;
    String wifiNum;
};

struct time_notifications {
    String timeSource;
    String epochTime;
};

//notifications module_noti;
sat_notifications sat_noti;
wifi_notifications wifi_noti;
time_notifications time_noti;

struct stepper_angles {
    float sat_angle;
    float wifi_angle;
    float rotation_angle;
};

stepper_angles angles;

void all_module_setup();
void get_module_notifications();
void display_init_page(sat_notifications _satNoti, wifi_notifications _wifiNoti); 
void update_time();
void update_gps();
void update_sats();
void update_Wifi();
void update_display();
void update_angle_display();
void update_angle();
void update_stepper();





