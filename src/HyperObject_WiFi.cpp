#include "HyperObject_WiFi.h"

HO_WiFi::HO_WiFi() {}

void HO_WiFi::WiFi_setup() {
  WiFi.mode(WIFI_STA);
  wifiNoti = "WIFI STA";
  configTime(0, 0, ntpServer);
}

void HO_WiFi::WiFi_update() {
  magWifi = 0; //signal magnitude reset

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

  if (n != 0) {
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      //Serial.print(i + 1);
      //Serial.print(": ");
      //Serial.print(WiFi.SSID(i));
      //Serial.print(" (");
      //Serial.print(WiFi.RSSI(i));
        
      magWifi += 100 + WiFi.RSSI(i);
      delay(5);
    }
    numWifi = n;
  }
}

int HO_WiFi::Get_numWifi() {
    int num;
    num = numWifi;
    return num;
}

int HO_WiFi::Get_magWifi() {
    int mag;
    mag = magWifi;
    return mag;
}

String HO_WiFi::Get_notiWifi() {
  String notification;
  notification = wifiNoti;

  return notification;
}

String HO_WiFi::Get_WifiNum() {
  String notification;
  notification = String(numWifi);

  return notification;
}