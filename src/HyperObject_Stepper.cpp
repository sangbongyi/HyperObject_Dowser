#include "HyperObject_Stepper.h"

Stepper::Stepper() 
  : stepper1(AccelStepper::HALF4WIRE, motor_1_Pin1, motor_1_Pin3, motor_1_Pin2, motor_1_Pin4),
    stepper2(AccelStepper::HALF4WIRE, motor_2_Pin1, motor_2_Pin3, motor_2_Pin2, motor_2_Pin4) {}


void Stepper::setup() {
  stepper1.setMaxSpeed(500.0);
  stepper1.setCurrentPosition(0);
  stepper1.setAcceleration(150.0);
  stepper1.setSpeed(30);
  
  stepper2.setMaxSpeed(500.0);
  stepper2.setCurrentPosition(0);
  stepper2.setAcceleration(150.0);
  stepper2.setSpeed(30);
}

/*int Stepper::Angle_update(int _numWifi, int _magWifi) {
  int rotationAngle;
  rotateAngleWifi = Get_wifi_angle(_numWifi, _magWifi);
  rotateAngleSat  = Get_sat_angle();
  
  rotationAngle = Get_rotation_angle(rotateAngleWifi, rotateAngleSat);

  return rotationAngle;
}*/

void Stepper::update(int _dstAngle) {
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

int Stepper::Get_wifi_angle(int _num, int _mag) {
  int angle = 0;
  angle = _num + abs(_mag / _mag);
  angle = constrain(angle, 0, 15);

  return angle;
}

int Stepper::Get_sat_angle(float * _sat_vis, float * _sat_ele, float * sat_dist) {
  int numberSat = 6;
  
  float eleAngle;
  float distAngle;
  float visAngle;
  
  int angle = 0;

  for (int i = 0; i < numberSat; i++) {
    if(_sat_vis[i] == -2) 
    { 
      //Under horizon 
      visAngle += (_sat_vis[i] * 10); // -20
      eleAngle += 0;
      distAngle += 0;
    } 
    else if(_sat_vis[i] == -1)
    {
      //Day light
      visAngle += (_sat_vis[i] * 10); // -10
      eleAngle += (180 + _sat_ele[i] / 100 * numberSat);  // Positive number  180 + (-180 - 180)
      distAngle += (35000 - sat_dist[i]  / 35000 * numberSat);
    }
    else
    {
      //Visible
      visAngle += (_sat_vis[i] / 100); // 0 - 1000
      eleAngle += (180 + _sat_ele[i] / 100 * numberSat);  // Positive number  180 + (-180 - 180)
      distAngle += (35000 - _sat_ele[i]  / 35000 * numberSat);
    }
  }
  angle = eleAngle + distAngle + visAngle;
  angle = constrain(angle, 0, 60);
  
  return angle;
}

int Stepper::Get_rotation_angle(int _angleWifi, int _angleSat) {
  int rotation;
  rotation = _angleWifi + _angleSat;

  return rotation;
}

float Stepper::Get_angle_to_step(float _angle) {
  float stepperStep;
  stepperStep = round(_angle * ONE_REVOLUTION / 360);
  
  return stepperStep;
}