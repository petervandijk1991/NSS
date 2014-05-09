#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);

//
// Topology
//
// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL};

bool listen_receiver = true;

void setup(void)
{  
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/pingpair/\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();
  radio.setRetries(15,15);
  radio.setChannel(48);
  radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)
  // De bovenste zijn connectie met de receiver, de onderste twee met de sender. 
  
    //luisteren naar de Sender of deze iets stuurt
    radio.openReadingPipe(2,pipes[0]);
    
    //radio.openWritingPipe(pipes[2]);
    //radio.openReadingPipe(1,pipes[3]);
  
   // radio.openWritingPipe(pipes[1]);
    
  

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
}
void loop(void)
{
  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //
  printf("Ik ben de hop");
  if(listen_receiver){
    radio.openReadingPipe(2,pipes[0]);
  } else {
    radio.openReadingPipe(1,pipes[3]);
  }
 
  // if there is data ready
  printf("Is de radio vrij?");
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    unsigned long message;
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &message, sizeof(unsigned long) );
      
      // Spew it
      printf("Wat is de oayload??");
      printf("Got payload %lu...",message);
      
      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(20);
    }
    
    // First, stop listening so we can talk
    radio.stopListening();
    if(listen_receiver){
      radio.openWritingPipe(pipes[2]);
    } else {
      radio.openWritingPipe(pipes[1]);
    }
    // Send the final one back.
    radio.write( &message, sizeof(unsigned long) );
    printf("Sent response.\n\r");
    
    // Now, resume listening so we catch the next packets.
    radio.startListening();
  }
  
  if(listen_receiver){
    listen_receiver = false;
  }else{ 
    listen_receiver = true; 
  }
}