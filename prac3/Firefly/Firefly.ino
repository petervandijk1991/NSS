#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


/*****************************************************
 *                                                   *
 *                   CONFIGURATION                   *
 *                                                   *
 *****************************************************/
 //ID+DELIM+highestID+DELIM+(millis()-ijkpunt)
 //Struct

 
 
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
  long delta_t;    //verschil tussen ontvangen tijd en onze tijd
  unsigned long ijkpunt;    //begin van de synchronisatie fase
  boolean light_on = false;
  int send_max = 5; //aantal pakketten wat verzonden wordt voor een nieuwe synchronisatie ronde plaatsvind
  
  int sleep = 5000; //sleep time in ms
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
  highestID = ID;
  
  printf_begin();
  printf("DE ID VAN DEZE NODE IS: %i ",ID);
  
  radio.begin();
  radio.openWritingPipe(pipes[0]);
  printf("Pipes openene gaat goed");
  radio.openReadingPipe(1,pipes[0]);
  printf("De eerste pipe gaat goed");
  radio.setRetries(0,0);
  radio.setPayloadSize(sizeof(MESSAGE));
  radio.printDetails();
  radio.startListening();
}

//Houdt de hoogste ID node bij en verandert deze als dat nodig is. 

void processContent(int sender, int high , long time){
   if(high >highestID){//If we receive a higher ID than our highest yet received
      highestID = high;
      senderID  = sender;
      delta_t = time;  
   } 
}

//verwerkt het bericht en leest de sender, highest en timestamp
//als dit niet werkt kunnen we een struct gebruiken
//Het verschil tussen [] en * is dat het eerste echt een array is zoals in java
//en * is een pointer zoals in C met adres en shizzle
void processMessage(MESSAGE ontvangen){ 
  printf("De methode process message!");
  int sender = ontvangen.identity;
  int highest = ontvangen.high_identity;
  long time = ontvangen.time_delay;
 // sender  = strtok_r(message, ""+DELIM, &p);
  //highest = strtok_r(NULL, ""+DELIM, &p);
  //time    = strtok_r(NULL, ""+DELIM, &p);
  printf("de sender heeft ID: %i", sender);
  printf("ontvangen data= \n\r sender: %i HIGH: %i, time: %ul \n\r ", sender, highest, time);
  processContent(sender, highest, time);
}

  //
  //  Luisteren zo lang als de ID is om niet te gaan zenden als anderen al aan het zenden zijn
  //
void listenFor(long time){
  int t = millis();
  //radio.startListening();
  MESSAGE ontvangen;
  while(millis()-t < time){
    if(radio.available()){
      radio.read( &ontvangen, sizeof(MESSAGE));
      processMessage(ontvangen);
    }
  } 
  //radio.stopListening(); 
}
 
void sendMessage(){ 
  radio.stopListening();
  MESSAGE bericht = { 3, 6, (millis()-ijkpunt)};
  printf("Op dit moment moet er een bericht gestuurd worden :) \n\r");
  printf("het bericht is: hihest ID = %i \n\r", highestID );
  radio.write(&bericht, sizeof(MESSAGE));
  radio.startListening();
}

void synchronize(){
  ijkpunt = millis();
  int delay_time = ID/sync_messages;
  int i = sync_messages;
  //de eerste nacht
  while( i != 0){
    listenFor(delay_time);
    sendMessage();  
    listenFor((sleep/sync_messages)-delay_time);
    i--;
  }
  //de tweede nacht
  int temp = sleep-delta_t;
  delay(temp);
  sendSignal();
  
}

void sendSignal(){
   //TODO: laat licht knipperen
  if(light_on){
     light_on = false; 
    digitalWrite(led, LOW);   // turn the LED off (LOW is the voltage level)
    printf("led staat uit en millis = %ul \n\r", millis());

  }else{
    light_on = true;
    
    digitalWrite(led, HIGH);    // turn the LED on by making the voltage HIGH
    printf("Led staat aan + millis = %ul \n\r", millis());
  }
  sendMessage();
}

void loop(void)
{
  synchronize();
  int i=1;
  while(i < send_max){
      delay(sleep);
      sendSignal();
      i++;
  }  
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
