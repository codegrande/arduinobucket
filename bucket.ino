#include "Arduino.h"
#include <avr/sleep.h>
#include <avr/power.h>
#include <avr/wdt.h>

//bucket is based on the code provided by 

//Put all the pins in an array to make them easy to work with
int pins[] {
    2,  //IN1 on the ULN2003 Board, BLUE end of the Blue/Yellow motor coil
    3,  //IN2 on the ULN2003 Board, PINK end of the Pink/Orange motor coil
    4,  //IN3 on the ULN2003 Board, YELLOW end of the Blue/Yellow motor coil
    5   //IN4 on the ULN2003 Board, ORANGE end of the Pink/Orange motor coil
};

//Define the wave drive sequence.  
//With the pin (coil) states as an array of arrays
int waveStepCount = 4;
int waveSteps[][4] = {
    {HIGH,LOW,LOW,LOW},
    {LOW,HIGH,LOW,LOW},
    {LOW,LOW,HIGH,LOW},
    {LOW,LOW,LOW,HIGH}
  };

//Keeps track of the current step.
//We'll use a zero based index. 
int currentStep = 0;

//samplingTime is expressed in ENTIRE minutes, no fractions
int samplingTime=1;

//capture is used to keep track of the number of test tubes we are using. This should match the targetSteps
int captura = 0;

//tubes is the number of tubes we have in the bucket
int tubes = 0;

//Keeps track of the current direction
//Relative to the face of the motor. 
//Clockwise (true) or Counterclockwise(false)
//We'll default to clockwise
bool clockwise = true;

int revolutionSteps = 2048;  //2049 steps per rotation when wave or full stepping
//int targetSteps = 4096;  //4096 steps per rotation when half stepping

//128 steps to move to the next test tube when wave or full stepping for 16 tubes
//256 steps to move to the next test tube when wave or full stepping for 8 tubes
int targetSteps = 256;  //here configured for 8 tubes

void demora (int minutos) {
  // we use 2 calls to delay with 30000 milliseconds (30 seconds) for every minute:
  
  for(int min = 0; min < minutos; min++) {
    delay(30000);
    delay(30000);
  }
}

void setup() {
  // put your setup code here, to run once:
  demora(samplingTime);
  Serial.begin(9600);
  tubes = 2048 / targetSteps;
  
  for(int pin = 0; pin < 4; pin++) {
    pinMode(pins[pin], OUTPUT);
    digitalWrite(pins[pin], LOW);
  }
}

void step(int steps[][4], int stepCount) {
  //Then we can figure out what our current step within the sequence from the overall current step
  //and the number of steps in the sequence
  int currentStepInSequence = currentStep % stepCount;
  
  //Figure out which step to use. If clock wise, it is the same is the current step
  //if not clockwise, we fire them in the reverse order...
  int directionStep = clockwise ? currentStepInSequence : (stepCount-1) - currentStepInSequence;  

  //Set the four pins to their proper state for the current step in the sequence, 
  //and for the current direction
  for(int pin=0; pin < 4; pin++){
    digitalWrite(pins[pin],steps[directionStep][pin]);
  }  
}

void loop() {
 
  //Comment out the Serial prints to speed things up
  //Serial.print("Step: ");
  //Serial.println(currentStep);
  
  //Get a local reference to the number of steps in the sequence
  //And call the step method to advance the motor in the proper direction
  int stepCount = waveStepCount;
  step(waveSteps,waveStepCount);
    
  // Increment the program field tracking the current step we are on
  ++currentStep;
  
  // If targetSteps has been specified, and we have reached
  // that number of steps, reset the currentStep, and reverse directions
  if(targetSteps != 0 && currentStep == targetSteps){
    
    currentStep = 0;

    //Here we increment captura and then check if we are done sampling. 
    //If we are done, we shut down to save power and keep the samples untainted.
    ++captura;
    if (captura == tubes) {
      set_sleep_mode(SLEEP_MODE_PWR_DOWN);
      cli();  // Disable interrupts
      sleep_mode();
    }
    demora(samplingTime);
    
  } else if(targetSteps == 0 && currentStep == stepCount) {
    // don't reverse direction, just reset the currentStep to 0
    // resetting this will prevent currentStep from 
    // eventually overflowing the int variable it is stored in.
    currentStep = 0;
  }
  
  //2000 microseconds, or 2 milliseconds seems to be 
  //about the shortest delay that is usable.  Anything
  //lower and the motor starts to freeze. 
  //delayMicroseconds(2250);
  delay(2);
}
