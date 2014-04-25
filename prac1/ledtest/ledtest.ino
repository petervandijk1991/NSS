

#include <I2C.h>
#include "MMA845.h"

int green = 13;
int blue = 12;
int lblue = 11;
int red = 10;
int yellow = 9;

// the setup routine runs once when you press reset:
void setup() {                
  // initialize the digital pin as an output.
  pinMode(green, OUTPUT);     
  pinMode(red, OUTPUT);     
  pinMode(blue, OUTPUT);     
  pinMode(lblue, OUTPUT);     
  pinMode(yellow, OUTPUT);     
}

// the loop routine runs over and over again forever:
void loop() { 
  digitalWrite(yellow, LOW); 
  digitalWrite(green, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               
  digitalWrite(green, LOW); 
  digitalWrite(blue, HIGH);  
  delay(1000);               
  digitalWrite(blue, LOW); 
  digitalWrite(lblue, HIGH);  
  delay(1000);               
  digitalWrite(lblue, LOW); 
  digitalWrite(red, HIGH);  
  delay(1000);               
  digitalWrite(red, LOW); 
  digitalWrite(yellow, HIGH);    
  delay(1000);        
}
