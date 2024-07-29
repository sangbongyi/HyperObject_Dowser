#include "HyperObject_Satellite.h"

Sat::Sat() {
    
}
// GPS functions
void Sat::GPS_setup() {
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

void Sat::GPS_update() {
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

// Sat functions
// Public functions
void Sat::Sat_init() {
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

void Sat::Sat_update() {
    Sat_get_tracks();
    Sat_get_ticks();
    Sat_get_tele();
}

//Private functions
void Sat::Sat_get_tracks() {
  // findsat
  for(int i = 0; i < 6; i++) {
    satellites[i]->findsat(epochTime);
  }
}

void Sat::Sat_get_ticks () {
    current_tr_act_sat_num = 0;
    for (int i = 0; i < 6; i++) {
        Sat_tick(*satellites[i]);
    }
}

void Sat::Sat_get_tele() {
  // Get values from SAT instances and update
  for (int i = 0; i < 6; i++) {
    sat_ele[i] = satellites[i]->satEl;
    sat_dist[i] = satellites[i]->satDist;
    sat_vis[i] = satellites[i]->satVis;
  }
}

void Sat::Sat_setup(Sgp4 &_sat, Sat_TrackInfo &_sat_info) {
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

void Sat::Sat_tick(Sgp4 &_sat) {
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

String Sat::Get_notiGpsData() {
  String notification;
  notification = gpsDataNoti;

  return notification;
}
        
String Sat::Get_notiGpsLoc() {
  String notification;
  notification = gpsLocNoti;

  return notification;
}

String Sat::Get_notiSat() {
  String notification;
  notification = satNoti;

  return notification;
}

String Sat::Get_gpsNum() {
  String notification;
  notification = String(current_gps_sat_num);

  return notification;
}

String Sat::Get_satNum() {
  String notification;
  notification = String(current_tr_act_sat_num);

  return notification;
}

String Sat::Get_timesource() {
  String notification;
  notification = timeSource;

  return notification;
}

String Sat::Get_epochTime() {
  String notification;
  notification = String(epochTime);

  return notification;
}

//============================ Calculate time functions ========================//
void Sat::GPS_time_update() {
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

int Sat::Cal_unix_time() {
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


