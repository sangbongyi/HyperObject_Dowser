#ifndef HYPEROBJECT_SATELLITE
#define HYPEROBJECT_SATELLITE

#include <Arduino.h>
#include <Sgp4.h>
#include <TinyGPS++.h>

#define MYLAT 53.1061864 //dummy
#define MYLON 8.8643598
#define MYALT 13
#define MYUNIXTIME 1618431790
#define INTERVAL 3000

class Sat {
    public:
        //================Satellite tracking information================//
        struct Sat_TrackInfo {
            String sat_satname;
            String sat_tle_line1;
            String sat_tle_line2;
        };
                
        Sat();
        // GPS functions
        void GPS_setup();
        void GPS_update();
        void GPS_time_update();

        // Sat functions
        void Sat_init();
        void Sat_update();

        String Get_notiGpsData();
        String Get_notiGpsLoc();
        String Get_notiSat();
        String Get_gpsNum();
        String Get_satNum();
        String Get_timesource();
        String Get_epochTime();

        float sat_ele[6];
        float sat_dist[6];
        float sat_vis[6];

    private:
        void Sat_setup(Sgp4 &_sat, Sat_TrackInfo &_sat_info);
        void Sat_tick(Sgp4 &_sat);
        void Sat_get_tele();
        void Sat_get_ticks();
        void Sat_get_tracks();
        int Cal_unix_time();
        // GPS Instance
        TinyGPSPlus gps;

        // GPS notification string
        String gpsDataNoti;
        String gpsLocNoti;
        String timeSource;
        
        float Flat, Flon, Alt, timeGps;
        long timeSinceLastEpochUpdate = 0;
        
        //================Satellite configuratoin================//
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

        Sat_TrackInfo satInfo_1;
        Sat_TrackInfo satInfo_2;
        Sat_TrackInfo satInfo_3;
        Sat_TrackInfo satInfo_4;
        Sat_TrackInfo satInfo_5;
        Sat_TrackInfo satInfo_6;

        Sat_TrackInfo *satInfos[6] = {
            &satInfo_1, &satInfo_2, &satInfo_3, &satInfo_4, &satInfo_5, &satInfo_6
        };

        // Satellite notification string
        String satNoti;
        
        int current_gps_sat_num = 0; //gps sat number
        int current_tr_act_sat_num = 0; //tracking sat (active)

        int numberSat = 6;

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
};

#endif