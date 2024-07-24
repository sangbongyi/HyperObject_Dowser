#include <Arduino.h>
#include <Sgp4.h>
#include <U8g2lib.h>
#include <AccelStepper.h>
#include <TinyGPS++.h>
#include <SPI.h>
#include <WiFi.h>
#include <Wire.h>
#include "time.h"

#define ONE_REVOLUTION 4096

#define motor_1_Pin1  2                 // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_1_Pin2  17                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_1_Pin3  21                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_1_Pin4  22                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define motor_2_Pin1  23                // IN1 pin on the ULN2003A driver to pin D1 on NodeMCU board
#define motor_2_Pin2  25                // IN2 pin on the ULN2003A driver to pin D2 on NodeMCU board
#define motor_2_Pin3  32                // IN3 pin on the ULN2003A driver to pin D3 on NodeMCU board
#define motor_2_Pin4  33                // IN4 pin on the ULN2003A driver to pin D4 on NodeMCU board

#define MYLAT 53.1061864 //dummy
#define MYLON 8.8643598
#define MYALT 13
#define MYUNIXTIME 1618431790

#define INTERVAL 3000

TinyGPSPlus gps;

float Flat, Flon, Alt, timeGps;
long timeSinceLastEpochUpdate = 0;

// put function declarations here:
int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  int result = myFunction(2, 3);
}

void loop() {
  // put your main code here, to run repeatedly:
}

// put function definitions here:
int myFunction(int x, int y) {
  return x + y;
}