#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"


/*****************************************************
 *                                                   *
 *                   CONFIGATION                     *
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
  
  int sleep = 1000; //sleep time in ms
  int sync_messages = 5; //# of Sync messages sent per sync phase

void setup(void)
{
  Serial.begin(57600);
  
  //create random ID for this device
  randomSeed(analogRead(0));
  ID = random(minID, maxID);
  highestID = ID;
  
  printf_begin();
  printf("DE ID VAN DEZE NODE IS: %i ",ID);

  radio.begin();
  radio.setRetries(0,0);
  radio.setPayloadSize(8);
  radio.openReadingPipe(0,pipes[0]);
  radio.startListening();
  radio.printDetails();
}

//Houdt de hoogste ID node bij en verandert deze als dat nodig is. 

void processContent(char sender [], char high [], char time []){
   if(atoi(high) >highestID){//If we receive a higher ID than our highest yet received
      highestID = atoi(high);
      senderID  = atoi(sender);
      timestamp = atol(time);
   } 
}

//verwerkt het bericht en leest de sender, highest en timestamp
//als dit niet werkt kunnen we een struct gebruiken
//Het verschil tussen [] en * is dat het eerste echt een array is zoals in java
//en * is een pointer zoals in C met adres en shizzle
void processMessage(char message []){ 
  char *p;
  char *sender;
  char *highest;
  char *time;
  sender  = strtok_r(message, ""+DELIM, &p);
  highest = strtok_r(NULL, ""+DELIM, &p);
  time    = strtok_r(NULL, ""+DELIM, &p);
  processContent(sender, highest, time);
}

  //
  //  Luisteren zo lang als de ID is om niet te gaan zenden als anderen al aan het zenden zijn
  //
void listenFor(long time){
  int t = millis();
  while(millis()-t < time){
    radio.startListening();
    char *read;
    if(radio.available()){
      radio.read( read, sizeof(char));
      processMessage(read);
    }
  } 
  radio.stopListening();
}

  //  
  //
  //
void sendMessage(){
  radio.openWritingPipe(pipes[0]);
  messageBuffer = ""+ID+DELIM+highestID+DELIM+millis(); 
  radio.write(&messageBuffer, sizeof(messageBuffer));
}

void synchronize(){
  int delay = ID/sync_messages;
  int i = sync_messages;
  while( i != 0){
    listenFor(delay);
    sendMessage();  
    listenFor((sleep/sync_messages)-delay);
    i--;
  }
}



void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (highestID == ID)
  {
    // First, stop listening so we can talk.
    radio.stopListening();

    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    printf("Now sending %lu...",time);
    bool ok = radio.write( &time, sizeof(unsigned long) );
    
    if (ok)
      printf("ok...");
    else
      printf("failed.\n\r");

    // Now, continue listening
    radio.startListening();

    // Wait here until we get a response, or timeout (250ms)
    unsigned long started_waiting_at = millis();
    bool timeout = false;
    while ( ! radio.available() && ! timeout )
      if (millis() - started_waiting_at > 200 )
        timeout = true;

    // Describe the results
    if ( timeout )
    {
      printf("Failed, response timed out.\n\r");
    }
    else
    {
      // Grab the response, compare, and send to debugging spew
      unsigned long got_time;
      radio.read( &got_time, sizeof(unsigned long) );

      // Spew it
      printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
    }

    // Try again 1s later
    delay(1000);
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if (!(highestID == ID))
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      unsigned long got_time;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &got_time, sizeof(unsigned long) );

        // Spew it
        printf("Got payload %lu...",got_time);

	// Delay just a little bit to let the other unit
	// make the transition to receiver
	delay(20);
      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_time, sizeof(unsigned long) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
