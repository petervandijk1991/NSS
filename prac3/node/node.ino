#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

/*****************************************************
 *                                                   *
 *                   CONFIGURATION                   *
 *                                                   *
 *****************************************************/
  RF24 radio(3, 9);
  const uint64_t pipes =  0xF0F0F0F0E1LL;

  int minID = 0;
  int maxID = 1000;
  int ID;
  
  const char DELIM = ':';
  const char* messageBuffer;
  
  //Previous synchronisation data needed to detect loops
  int prevHighestID; //Previous Highest found ID
  int prevMessageID; //Previous message ID
  
  int highestID;  //Highest found ID
  int senderID;   //Sender of highest ID
  int messageID;  //MessageID
  long timestamp; //Timestamp of highestID
  long deltaT; 
  boolean lightOn = false;
  int sendMax = 5;  //amount of messages sent before synchronisation
  int messageCount; //amount of sent messages
  int night = 1000; //sleep time in ms
  
  //Set up LED
  int led = 7;

void setup(void){
  Serial.begin(57600);
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  //create random ID for this device
  randomSeed(analogRead(0));
  ID = random(minID, maxID);
  
  printf_begin();
  printf("NODE ID: %i \n\r",ID);
  
  radio.begin();
  radio.setRetries(0,0);
  radio.setPayloadSize(sizeof(MESSAGE));
  radio.openWritingPipe(pipes);
  radio.openReadingPipe(1,pipes);
  radio.startListening();
  radio.printDetails();
  
  messageCount = 0;
  sendMax++;
  prevHighestID = -1;
  prevMessageID = -1;
}



//
//  Listen for messages and process
//
void listenFor(long time){
  long t = millis();
  MESSAGE ontvangen;
  while(millis()-t < time){
    if(radio.available()){
      radio.read( &ontvangen, sizeof(MESSAGE));
      processMessage(ontvangen);
    }
  } 
}


void processMessage(MESSAGE received){ 
  processContent(received.identity, received.high_identity, received.number);
}

//
// process message and change highestID if necessary
//
void processContent(int sender, int high, int messNumber){
   printf("RECEIVED: %i, %i, %i\n\r", sender, high, messNumber);
   if((high >  highestID && !(prevMessageID == messNumber && prevHighestID == high))   //if we receive a higher ID than our highest yet received
   || (high == highestID && sender == high)                //or if we receive previously received highest from highest node itself
   ){                                                      //update highest
     deltaT   = millis()-timestamp;                        //Tijd die nodig is van luisteren naar het verwerken van het bericht wanneer een bericht is ontvangen
     printf("NEW HIGHEST!\n\r");
     highestID = high;
     senderID  = sender;
     messageID = messNumber;
   }
}

void sendMessage(){ 
  MESSAGE bericht = { ID, highestID, messageID };
  printf("SEND: %i, %i, %i: ", ID, highestID, messageID);
  radio.stopListening();
  bool ok = radio.write(&bericht, sizeof(MESSAGE));
  radio.startListening();
  if(ok){
    printf("SENT CORRECTLY\n\r"); 
  }else{
    printf("SENDING FAILED\n\r");
  }
  messageCount++;
}

void synchronize(){
  highestID = ID;           //reset data
  deltaT    = night;
  timestamp = millis();
  messageID = messageCount;
       
  listenFor(night);          //first night: find highest
  prevMessageID = messageID; //update prevMessageID
  prevHighestID = highestID; //update prevHighestID
  if(deltaT == night){       //if no new highest found
     sendSignal();
  }else{                     //toggle LED to indicate wake-up
    toggleLED();
  }
  printf("SLEEP FOR %ul", deltaT); 
  sleepFor(deltaT);         //second night: synchronise
  sendSignal();
}

void toggleLED(){
  if(lightOn){
     lightOn = false; 
    digitalWrite(led, LOW);  // turn the LED off (LOW is the voltage level)
  }else{
    lightOn = true;
    digitalWrite(led, HIGH); // turn the LED on by making the voltage HIGH
  }
}

void sendSignal(){
  toggleLED();
  sendMessage();
}

void loop(void){
  while((messageCount % sendMax) != 0){
    sleepFor(night);
    sendSignal();
  }  
  synchronize();
}

void sleepFor(long time){
 delay(time); 
}
