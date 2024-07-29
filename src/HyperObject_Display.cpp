#include "HyperObject_Display.h"

HO_Display::HO_Display() 
  :u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16) {}

void HO_Display::Display_setup() {
  u8g2.begin();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.clearDisplay();
  u8g2.setDisplayRotation(U8G2_R1);
  u8g2.setFlipMode(0);
}

void HO_Display::Display_draw_firstPage(const char *gps_data, const char *gps_loc, const char *wifi_init, const char *sat_init) {
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

void HO_Display::Display_draw(const char *s) {
  u8g2.firstPage();
  do {  
    u8g2.drawStr(2,15,s); 

  } while (u8g2.nextPage());
  delay(500);
}

void HO_Display::Display_update(const char *gps_available, const char *time_source, const char *epoch_time, 
                                const char *wifi_num,      const char *g_sats_num,  const char *t_sats_num) {
  u8g2.clearDisplay();
  u8g2.firstPage();
  do {
    u8g2.drawStr(2,15, "Update...");    
    u8g2.drawStr(2,30, "G_LOC ");
    u8g2.drawStr(40,30, gps_available); 

    u8g2.drawStr(2,45, "T: ");
    u8g2.drawStr(15,45, time_source);
    
    u8g2.drawStr(2,60, epoch_time);    
    
    u8g2.drawStr(2,75, "WiFi ");
    u8g2.drawStr(30,75, wifi_num);
    
    u8g2.drawStr(2,90, "GPS SAT ");
    u8g2.drawStr(45,90, g_sats_num);

    u8g2.drawStr(2,105, "TRK SAT ");
    u8g2.drawStr(45,105, t_sats_num);
    
  } while ( u8g2.nextPage() );
  delay(3000);
}

void HO_Display::Display_rotation_angle(const char *gps_Angle, const char *wifi_Angle, const char *rot_angle) {
  u8g2.clearDisplay();
  u8g2.firstPage();
  do {
    u8g2.drawStr(2,15, "Rotate...");    
    
    u8g2.drawStr(2,30, "GPS ");
    u8g2.drawStr(30,30, gps_Angle); 
    
    u8g2.drawStr(2,45, "WIFI "); 
    u8g2.drawStr(30,45, wifi_Angle);    

    u8g2.drawStr(2,60, "A: "); 
    u8g2.drawStr(30,60, rot_angle);  
    
  } while ( u8g2.nextPage() );
}