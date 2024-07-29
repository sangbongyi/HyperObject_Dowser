#ifndef HYPEROBJECT_WiFi
#define HYPEROBJECT_WiFi

#include <Arduino.h>
#include <WiFi.h>

class HO_WiFi {
    public:
        HO_WiFi();
        void WiFi_setup();
        void WiFi_update();
        int Get_numWifi();
        int Get_magWifi();
        
        String Get_notiWifi();
        String Get_WifiNum();

    private:
        // WiFi notification string
        String wifiNoti;

        //================Wifi configuratoin================//
        const char* ssid       = "TP-Link_F3E0";
        const char* password   = "45876158";
        const char* ntpServer = "pool.ntp.org";

        int numWifi = 1;
        int magWifi = 0;
};

#endif