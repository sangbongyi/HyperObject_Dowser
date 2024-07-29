#ifndef HYPEROBJECT_STEPPER
#define HYPEROBJECT_STEPPER

#include <Arduino.h>
#include <AccelStepper.h>

#define ONE_REVOLUTION 4096

#define motor_1_Pin1  2                 // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_1_Pin2  17                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_1_Pin3  21                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_1_Pin4  22                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define motor_2_Pin1  23                // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_2_Pin2  25                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_2_Pin3  32                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_2_Pin4  33                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define INTERVAL 3000

class Stepper {
    public:
        Stepper();
        // Stepper functions
        void setup();
        void update(int _dstAngle);
        // Get stepper angle functions
        int Get_wifi_angle(int _num, int _mag);
        int Get_sat_angle(float * _sat_vis, float * _sat_ele, float * sat_dist);


    private:
        // Variables for the steppers
        float steps_1;
        float steps_2;
        boolean initStepper = false;
        
        int Get_rotation_angle(int _angleWifi, int _angleSat);
        float Get_angle_to_step(float _angle);

        //================Stepper angle variables================//
        // Define some steppers and the pins the will use
        AccelStepper stepper1;
        AccelStepper stepper2;
        
        int rotateAngleWifi = 0;
        int rotateAngleSat = 0;
        int previousAngle = 0;

};
#endif