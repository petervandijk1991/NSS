

#include <I2C.h>
#include "MMA845.h"

MMA845 mma;

//        YELLOW                        MMA
//  BLUE  LBLUE    GREEN   
//        RED

int green = 13;
int blue = 12;
int lblue = 11;
int red = 10;
int yellow = 9;

void setup() {                
  pinMode(green, OUTPUT);     
  pinMode(red, OUTPUT);     
  pinMode(blue, OUTPUT);     
  pinMode(lblue, OUTPUT);     
  pinMode(yellow, OUTPUT);     
  Serial.begin(115200);
  mma.begin();
}

// the loop routine runs over and over again forever:
void loop() { 
  
  int x = 0, y = 0, z = 0;
  mma.getAccXYZ(&x, &y, &z);
  if(x<-25){
    digitalWrite(blue, HIGH);   
    digitalWrite(green, LOW); 
    digitalWrite(lblue, LOW); 
  }else if(x>25){
    digitalWrite(green, HIGH);   
    digitalWrite(blue, LOW); 
    digitalWrite(lblue, LOW); 
  }else{
    digitalWrite(lblue, HIGH); 
    digitalWrite(green, LOW); 
    digitalWrite(blue, LOW); 
  }
  if(y<-25){
    digitalWrite(red, HIGH);   
    digitalWrite(yellow, LOW); 
    digitalWrite(lblue, LOW); 
  }else if(y>25){
    digitalWrite(yellow, HIGH);   
    digitalWrite(red, LOW); 
    digitalWrite(lblue, LOW); 
  }else{
    digitalWrite(lblue, HIGH); 
    digitalWrite(yellow, LOW); 
    digitalWrite(red, LOW); 
  }  
  delay(10);        
}
