# stepper-code-
Assumes a 200-step motor (1.8° per step).  Sets speed to 100 RPM.  In loop(), it:  Rotates the motor one full revolution (200 steps).  Immediately repeats forever (because delay(0) does nothing).

#include <Stepper.h>

const int stepsPerRevolution = 200;

Stepper myStepper(stepsPerRevolution, 8, 9, 10, 11);

void setup() {
  myStepper.setSpeed(100);   // 100 RPM
  Serial.begin(9600);
}

void loop() {
  myStepper.step(stepsPerRevolution);
  delay(0);
}
