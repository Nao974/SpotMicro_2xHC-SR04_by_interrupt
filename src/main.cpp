/*
***********************
*/

#include <Arduino.h>

// Libraries for servo control
#include <Servo.h>
#include <Oscillator.h>
#include <choreograph.h>

// All declarations for 2x HC-SR04 sensors
// All the explanations here: https://github.com/Robot-Dog-2021/R-D_2xHC-SR04_by_interrupt
#include <hc-sr04.h>


const uint8_t  NBRE_SERVO=12;
const uint8_t  MAX_POS=180; // Servo Max 180°
const uint16_t TIME = 1000; // Home to Down in 1000 ms
const uint16_t FRAME = 10;  // Time divider to limit steps every 10 ms

// Configuration of the various servo motors
// Settings and positions made from the Choreograph program: 
// https://github.com/Nao974/choreograph-git
uint8_t pin[]={28, 26, 24, 36, 34, 32, 44, 42, 40, 52, 50, 48};
int8_t offset[]={-2, -2, 0, -2, -4, -11, 1, -10, -2, -10, -2, -9};
uint8_t orientation[]={'d', 'd', 'd', 'i', 'i', 'i', 'd', 'd', 'd', 'i', 'i', 'i'};
uint8_t angle_min[]={10, 2, 5, 10, 2, 5, 10, 2, 5, 10, 2, 5};
uint8_t angle_max[]={150, 178, 175, 150, 178, 175, 150, 178, 175, 150, 178, 175};

// Definition of the 2 positions
uint8_t pos_home[]={90,140,50,90,140,50,90,166,90,90,160,90};
uint8_t pos_down[]={90,150,30,90,150,30,90,178,10,90,178,10};
choreograph chore;

float decrement[NBRE_SERVO];
uint16_t iteration;
int8_t direction;

uint32_t next_time; // variable to replace the delay () blocking function

// Servo movement according to its orientation and min / max limit
void moveSingle_safe(uint8_t servo_id, uint8_t pos) {
    if (orientation[servo_id] == 'i')
        pos= MAX_POS -pos;
    if (pos <= angle_max[servo_id] && pos >= angle_min[servo_id])
        chore.moveSingle(servo_id, pos);
  }

// Initialization of all servos
void position_init(void) {
    for(uint8_t servo_id=0; servo_id<NBRE_SERVO; servo_id++) {
        chore.servoInit(pin[servo_id]);
        chore.servoOffset(servo_id, offset[servo_id]);
        moveSingle_safe(servo_id, pos_home[servo_id]); 
    }
  }

void setup() {
    PCICR  |= B00000010;  // Activation of the interrupt on port J
    PCMSK1 |= B00000110;  // Activation only of pin 14 & 15 (PJ1 & PJ0)

    position_init(); // to home position

    // Calculates the difference between the 2 positions for each servo 
    // depending on the time desired to switch from one to the other
    for (uint8_t i=0; i< NBRE_SERVO; i++)
        decrement[i] = double(pos_down[i] - pos_home[i]) / (TIME / FRAME);

    iteration = 0;  
    direction = 0;  
    delay(5000); // Just a delay for the installation of the robot after starting
    next_time = millis();
}

void loop() {
    // If the distance measurement is finished, calculate in cm 
    // and minimum between left and right    
    if (finished) {
        finished= false;
        if (!side) {
            right.range= CalcRange();
            if (right.range < left.range) 
                range = right.range;
            left.start();
        }
        else {
            left.range= CalcRange();
            if (left.range < right.range) 
                range = left.range;
            right.start();        
         }
        side = !side;
    }

    if (millis() >= next_time){ // Allows you to remove the delay () blocking function
        // Choice forwards or backwards according to the measured distance
        if ( range < 20 ) direction = 1;
        else direction = -1;

        // Limitation to not exceed the min / max positions
        if ( ( (iteration + direction) < (TIME / FRAME) ) && ( (iteration + direction) >= 0) ) iteration += direction;

        // Displacement of each servo according to calculated position
        for (uint8_t i=0; i< NBRE_SERVO; i++)
            moveSingle_safe(i, pos_home[i] + ( decrement[i] * iteration) );
        next_time = millis() + FRAME; // every FRAME ms
    }
}
