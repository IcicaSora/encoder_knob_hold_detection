//for this to work you need to isolate the top metal from the pcb ground
//on my omron encoder this is the case of just removing one grounded screw connecting to the metal on the inside

float A_pressThreshold = 0.85;    //range: 0.0<value<1.0   //when capsense value is less (baseline * pressThreshold), it's consider as pressed
float A_releaseThreshold = 1.1;   //range: value>1.0       //when capsense value is higher than (baseline * pressThreshold * releaseThreshold), it's consider as released 
                                  // if (baseline * pressThreshold * releaseThreshold) > baseline, release value will be the same as baseline

#define EncoderPPR 360
                                  
//#define WaitForSerial
//#define ResetAxisWhenNotTouching
//#define PrintMpr121

/* --------------------------------- */

#define enc_cooldown_const 500
int enc_cooldown = 0;
#define print_cooldown_const 1
int print_cooldown = 0;

#define sample_amount 10
int baseline=0;

int sensor[10];

#include <Joystick.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  1, 0,                  // Button Count, Hat Switch Count
  false, false, true,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

#include <Encoder.h>
Encoder myEnc(1, 0);
long oldPosition  = -999;

#include <Wire.h>
#include "Adafruit_MPR121.h"
Adafruit_MPR121 cap = Adafruit_MPR121();

void setup() {
  
  for (int i=0; i<20; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  Serial.begin(9600);

  #ifdef WaitForSerial
    while (!Serial) { // needed to keep leonardo/micro from starting too fast!
      delay(10);
    }
  #endif
  
  Serial.println("Adafruit MPR121 Capacitive Touch sensor test"); 
  if (!cap.begin(0x5A)) {
    Serial.println("MPR121 not found, check wiring?");
    while (1);
  }
  Serial.println("MPR121 found!");
  delay(100); //needed or else the first few values would be 0 and the calibration will be incorrect
  MPR121_Calibration();

  Joystick.begin();
  Joystick.setZAxisRange(0, 359);
}

void loop() {

  MPR121_detect();
  
  //encoder
    long newPosition = myEnc.read();
    oldPosition = newPosition;
    //Serial.println(newPosition);
    if (newPosition > EncoderPPR*4){myEnc.write(0);}
    if (newPosition < 0){myEnc.write(EncoderPPR*4);}
    Joystick.setZAxis(map(newPosition,0,EncoderPPR*4,0,359));

  
  if (print_cooldown>0){
    print_cooldown--;
    }
  if (enc_cooldown>0){
    enc_cooldown--;
    }
    
  if (print_cooldown<=0){
    #ifdef PrintMpr121
      Serial.println(cap.filteredData(0));
    #endif
    
    print_cooldown=print_cooldown_const;
  }

  if (enc_cooldown<=0){
    #ifdef PrintMpr121
      Serial.println(cap.filteredData(0));
    #endif

    #ifdef ResetAxisWhenNotTouching
      myEnc.write(0);
    #endif
    
    enc_cooldown=enc_cooldown_const;
  }
}

void MPR121_Calibration() {
  int baseline_buffer[sample_amount];    //take samples
  for (int i = 0; i<sample_amount; i++){
    baseline_buffer[i]=cap.filteredData(0);
    Serial.print(cap.filteredData(0));
    Serial.print(" | ");
  }
  Serial.println("");

  for (int i = 0; i<sample_amount; i++){
    baseline=baseline+baseline_buffer[i]; //add all samples then divide it by number of samples
  }
  baseline=baseline/sample_amount; //average number
  
  if (baseline*A_pressThreshold*A_releaseThreshold > baseline){
    A_releaseThreshold=1/A_pressThreshold;  //prevent release value from going over baseline
  }
    
  Serial.print("baseline: "); Serial.println(baseline);
  Serial.print("press: "); Serial.println(baseline*A_pressThreshold);
  Serial.print("release: "); Serial.println(baseline*A_pressThreshold*A_releaseThreshold);
  
}

void MPR121_detect() {
  if (cap.filteredData(0) < baseline * A_pressThreshold){
    sensor[0]=1;
    Joystick.setButton(0, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    enc_cooldown=enc_cooldown_const;
  }
  if (cap.filteredData(0) > baseline * A_pressThreshold * A_releaseThreshold && sensor[0]==1){
    sensor[0]=0;
    Joystick.setButton(0, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }
}
