//this exploits the noise generated when you are holding onto the encoder knob to achieve hold detection, yeah this is janky

//input the PPR value according to your encoder hardware
//higher the encoder PPR, better the hold detection
//i recommend using encoders equal or more than 600 PPR
#define EncoderPPR 360

#define knob_hold_timer 1000     //higher the PPR, lower this value can go, less delay so touch detection can release sooner
//having less friction on the knob also helps

#define noiseRange 2

#include <Encoder.h>
Encoder myEnc(1, 0);

// -- end of user configurables -- //


#include <Joystick.h>
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  1, 0,                  // Button Count, Hat Switch Count
  false, false, true,     // X and Y, but no Z Axis
  false, false, false,   // No Rx, Ry, or Rz
  false, false,          // No rudder or throttle
  false, false, false);  // No accelerator, brake, or steering

int knob_hold_timer_curr;

void setup() {
  Serial.begin(9600);
  Joystick.begin();
  Joystick.setZAxisRange(0, EncoderPPR*4);
}

long oldPosition  = -999;

void loop() {
  long newPosition = myEnc.read();

//hold detection
  if (newPosition-oldPosition!=0) {
    if (newPosition-oldPosition<=-noiseRange or newPosition-oldPosition<=noiseRange) {
      knob_hold_timer_curr = knob_hold_timer;
    }
  }

  if (knob_hold_timer_curr>0) {
    Joystick.setButton(0, HIGH);
    digitalWrite(LED_BUILTIN, HIGH);
    knob_hold_timer_curr--;
  }
  else {
    Joystick.setButton(0, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    }

//axis update
  if (newPosition != oldPosition) {
    oldPosition = newPosition;
    Serial.println(newPosition);
    Joystick.setZAxis(newPosition);
    if (newPosition > EncoderPPR*4) {myEnc.write(0);}
    if (newPosition < 0) {myEnc.write(EncoderPPR*4);}
  }
}