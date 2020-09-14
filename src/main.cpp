#include <Arduino.h>

#include <CapacitiveSensor.h>         //Library for fader touch sensitivity

#include <MIDI.h> //Library for receiving MIDI messages

//Arduino Pin Assignments
const int motorDown    = 8;   //H-Bridge control to make the motor go down
const int motorUp      = 9;   //H-Bridge control to make the motor go up

//Inputs
const int butt         = 28;
const int wiper        = PIN_A9;   //Position of fader relative to GND (Analog 0)
const int touchSend    = 19;   //Send pin for Capacitance Sensing Circuit (Digital 7)
const int touchReceive = 18;   //Receive pin for Capacitance Sensing Circuit (Digital 8)
//Variables
double faderMax        = 0;   //Value read by fader's maximum position (0-1023)
double faderMin        = 0;   //Value read by fader's minimum position (0-1023)
int    faderChannel    = 1;     //Value from 1-8

CapacitiveSensor touchLine     = CapacitiveSensor(touchSend, touchReceive);
volatile bool touched  = false; //Is the fader currently being touched?
bool     positionUpdated = false; //Since touching, has the MIDI position been updated?


//Calibrates the min and max position of the fader
void calibrateFader() {
    //Send fader to the top and read max position
    digitalWrite(10, HIGH);
    digitalWrite(motorUp, HIGH);
    delay(500);
    digitalWrite(motorUp, LOW);
    faderMax = analogRead(PIN_A9);

    //Send fader to the bottom and read max position
    digitalWrite(motorDown, HIGH);
    delay(500);
    digitalWrite(motorDown, LOW);
    faderMin = analogRead(PIN_A9);

}

//Selects the next channel in the DAW
void nextChannel() {
    static unsigned long last_interrupt0_time = 0;      //Interrupt Debouncing
    unsigned long interrupt0_time = millis();           //Interrupt Debouncing

    if (interrupt0_time - last_interrupt0_time > 200) { //Interrupt Debouncing
        if (faderChannel < 8) {
            faderChannel++;

            Serial.write(0x90);
            Serial.write(0x17 + faderChannel);
            Serial.write(0x7f);                         //Note On
            Serial.write(0x90);
            Serial.write(0x17 + faderChannel);
            Serial.write((byte) 0x00);                    //Note Off
        }
    }

    last_interrupt0_time = interrupt0_time;             //Interrupt Debouncing
}

void setup() {

    pinMode (motorUp, OUTPUT);
    pinMode (motorDown, OUTPUT);
    pinMode (10, OUTPUT);
    pinMode(28, INPUT_PULLUP);

    attachInterrupt(butt, nextChannel, FALLING);

    calibrateFader();
    Serial.begin(9600);
//    Serial.println("Hello Computer");

}
//Check to see if the fader is being touched
void checkTouch() {
    touched = touchLine.capacitiveSensor(30) >= 25;  //700 is arbitrary and may need to be changed
                                             //depending on the fader cap used (if any).
}


void loop() {

    int state = analogRead(PIN_A8);    //Read the state of the potentiometer
     checkTouch();
//    Serial.println(analogRead(PIN_A9));
//    Serial.println(digitalRead(butt));
    if (state < analogRead(PIN_A9) - 10 && state > faderMin) {
//        digitalWrite(10, HIGH);
        digitalWrite(motorDown, HIGH);
        while (state < analogRead(PIN_A9) - 10 ) {};  //Loops until motor is done moving
        digitalWrite(motorDown, LOW);
//        digitalWrite(10, LOW);
    }
    else if (state > analogRead(PIN_A9) + 10 && state < faderMax) {
        digitalWrite(motorUp, HIGH);
//        digitalWrite(10, HIGH);
        while (state > analogRead(PIN_A9) + 10) {}; //Loops until motor is done moving
        digitalWrite(motorUp, LOW);
//        digitalWrite(10, LOW);
    }
}
