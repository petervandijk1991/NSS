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
  const uint64_t pipes[1] = { 0xF0F0F0F0E1LL};

  int minID = 0;
  int maxID = 1000;
  int ID;
  
  const char DELIM = ':';
  const char* messageBuffer;
  
  int highestID;  //Highest found ID
  int senderID;   //Sender of highest ID
  long  timestamp;//Timestamp of highestID
  long delta_t; 
  unsigned long ijkpunt;    //begin van de synchronisatie fase
  boolean light_on = false;
  int send_max = 5; //aantal pakketten dat verzonden wordt voor een nieuwe synchronisatie ronde plaatsvindt
  
  int sleep = 1000; //sleep time in ms
  int sync_messages = 5; //# of Sync messages sent per sync phase
  
  //Set up LED
  int led = 7;

void setup(void)
{
  Serial.begin(57600);
  // initialize the digital pin as an output.
  pinMode(led, OUTPUT);
  //create random ID for this device
  randomSeed(analogRead(0));
  ID = random(minID, maxID);
  
  printf_begin();
  printf("DE ID VAN DEZE NODE IS: %i \n\r",ID);
  
  radio.begin();
  radio.setRetries(0,0);
  radio.setPayloadSize(sizeof(MESSAGE));
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
}




  //
  //  Luisteren zo lang als de ID is om niet te gaan zenden als anderen al aan het zenden zijn
  //
void listenFor(long time){
  int t = millis();
  MESSAGE ontvangen;
  while(millis()-t < time){
    if(radio.available()){
      radio.read( &ontvangen, sizeof(MESSAGE));
      processMessage(ontvangen);
    }
  } 
}


void processMessage(MESSAGE ontvangen){ 
  processContent(ontvangen.identity, ontvangen.high_identity, ontvangen.time_delay);
}

//Houdt de hoogste ID node bij en verandert deze als dat nodig is. 
void processContent(int sender, int high , long time){
   printf("RECE: %i, %i, %ul\n\r", sender, high, time);
   if(high >highestID){//If we receive a higher ID than our highest yet received
     printf("NEW HIGHEST!\n\r");
     timestamp = millis();
     highestID = high;
      senderID  = sender;
      delta_t = time;  
   } 
}

void sendMessage(){ 
  radio.stopListening();
  MESSAGE bericht = { ID, highestID, (millis()-ijkpunt)};
  printf("SEND: %i, %i, %ul: ", ID, highestID, (millis()-ijkpunt));
  bool ok = radio.write(&bericht, sizeof(MESSAGE));
  if(ok){
   printf("SENT CORRECTLY\n\r"); 
  }else{
   printf("SENDING FAILED\n\r");
  }
  radio.startListening();
}

void synchronize(){
  highestID = ID;
  delta_t = 0;
  ijkpunt = millis();
  timestamp = ijkpunt+sleep-1;
  //de eerste nacht
  listenFor(sleep);
  sendSignal();
  //de tweede nacht
  int temp = (timestamp-ijkpunt)%sleep;
  printf("SLEEP FOR %ul", temp);
  sleepFor(temp);
  sendSignal();
  
}

void sendSignal(){
  if(light_on){
     light_on = false; 
    digitalWrite(led, LOW);   // turn the LED off (LOW is the voltage level)
  }else{
    light_on = true;
    digitalWrite(led, HIGH);    // turn the LED on by making the voltage HIGH
  }
  sendMessage();
}

void loop(void)
{
  synchronize();
  int i=1;
  while(i < send_max){
      sleepFor(sleep);
      sendSignal();
      i++;
  }  
}

void sleepFor(long time){
 delay(time); 
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
