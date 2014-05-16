/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example RF Radio Ping Pair
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two different nodes,
 * connect the role_pin to ground on one.  The ping node sends the current time to the pong node,
 * which responds by sending the value back.  The ping node can then see how long the whole cycle
 * took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
// (MV:) Adapted to work with the configuration of the shield. Original: RF24 radio(9,10);

RF24 radio(3, 9);

 //
  // De range waartussen het random nummer moet liggen
  //
  int min = 0;
  int max = 1000;
  int ID = 0;
  char Delimiter = ":";
  boolean master = false;
  int Highest = 0;
  int sleep = 1000;
  int sync_messages = 5;
//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[1] = { 0xF0F0F0F0E1LL};



void setup(void)
{
  Serial.begin(57600);
  randomSeed(analogRead(0));
  ID = random(min, max);
  Highest = ID;
  printf("DE ID VAN DEZE NODE IS: %i ",ID);
  printf_begin();
  

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  //Retries would make synchronization impossible
  radio.setRetries(0,0);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
    
    //radio.openWritingPipe(pipes[0]);
    
  

  //
  // Start listening
  //

  radio.openReadingPipe(0,pipes[0]);
  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}

//Houdt de hoogste ID node bij en veranderd deze als dat nodig is. 

void declareHighest(char high []){
  int high = atoi(high);  
  if(Highest < high) {
     Highest = high; 
  }
}

void processMessage(char read []){
   char* chars_array = strtok(read, ":");
   sender_ID =  chars_array;
   char* chars_array = strtok(read, ":");
   highest_ID = chars_array;
   char* chars_array = strtok(read, ":");
   timestamp = chars_array;
   char* chars_array = strtok(read, ":");
   declareHighest(highest ID);
}

  //
  //  Luisteren zo lang als de ID is om niet te gaan zenden als anderen al aan het zenden zijn
  //
void listenFor(ul time){
   int t = millis();
  while(millis()-t < time){
     radio.startListening();
      char read [];
      radio.read( read, sizeof(char));
      processMessage(read);
  } 
  radio.stopListening();
}

  //  
  //
  //
void sendMessage(){
  radio.openWritingPipe(pipes[0]);
  char message [] = ""+ID+Delimiter+Highest+Delimiter+millis()+""; 
  radio.write(&message, sizeof(message));
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

  if (master)
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

  if (!master)
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
