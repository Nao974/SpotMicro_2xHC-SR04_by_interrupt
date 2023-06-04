#include<Arduino.h>

struct sr04 {
    uint8_t pin_trigger;
    uint8_t pin_echo;
    uint16_t range;

    // pin initialization
    sr04(uint8_t trigger, uint8_t echo) {
        pinMode(pin_trigger=trigger, OUTPUT);
        pinMode(pin_echo=echo, INPUT);
        range = 9999;
    }

    //Start of a measurement by going to the high state of the Trigger pin for 10µs
    void start() {
        digitalWrite(pin_trigger, LOW);
        delayMicroseconds(2);
        digitalWrite(pin_trigger, HIGH);
        delayMicroseconds(10);
        digitalWrite(pin_trigger, LOW);
    }
};

sr04  left(16,14); // (Trigger, Echo)
sr04 right(17,15);

uint8_t side=0; // 0 = Left, !0 = right
uint16_t range=9999;

int8_t message[16];

// variables modified in the ISR therefore in volatile
volatile uint32_t end_us=25000;
volatile uint32_t start_us=0;
volatile uint8_t finished=true;

/*
  The interrupt routine must be as short as possible, 
  Calculate the distance outside of it
*/
uint16_t CalcRange() { return ( (end_us - start_us) / 58 ); }

/*
 Interrupt routine when there is a change of state on pins 
 pin14 -> PJ1 -> PCINT10 \
                          > PCINT1 Vector
 pin15 -> PJ0 -> PCINT09 /
*/
ISR(PCINT1_vect) {          
    if ( PINJ & B00000011 ) // IF PJ0 OR PJ1 = HIGH 
        start_us=micros();  //    => ECHO signal start
    else {
        end_us=micros();    // ELSE ECHO signal end
        finished=true;      //    => acquisition completed
    }      
}