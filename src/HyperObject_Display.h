#ifndef HYPEROBJECT_DISPLAY
#define HYPEROBJECT_DISPLAY

#include <Arduino.h>
#include <U8g2lib.h>

class HO_Display {
    public:
        HO_Display();
        // Display functions
        void Display_setup();
        void Display_draw_firstPage(const char *gps_data, const char *gps_loc, 
                                    const char *wifi_init, const char *sat_init);
        void Display_draw(const char *s);
        void Display_update(const char *gps_available, const char *time_source, const char *epoch_time, 
                            const char *wifi_num, const char *g_sats_num, const char *t_sats_num);
        void Display_rotation_angle(const char *gps_Angle, const char *wifi_Angle, const char *rot_angle);
    private:
        U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8g2;
};

#endif
