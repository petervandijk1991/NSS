#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
int messages_sent = 0;

const uint64_t pipes[4] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0C3LL, 0xF0F0F0F0B4LL};
const uint8_t readingPipeSender = 1;
const uint8_t readingPipeReceiver = 2;
uint8_t receivedFrom;

bool listen_receiver = true;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/pingpair/\n\r");

  radio.begin();
  radio.setRetries(15,15);
  radio.setChannel(48);
  radio.setPayloadSize(255);
  radio.setPALevel(RF24_PA_MIN);
  radio.setDataRate(RF24_2MBPS);

  radio.openReadingPipe(readingPipeSender,pipes[0]);
  radio.openReadingPipe(readingPipeReceiver,pipes[3]);

  //
  // Start listening
  //
  radio.startListening();
  radio.printDetails();
}
void loop(void){
     //
  // Pong back role.  Receive each packet, dump it out, and send it back
  // 
  // Message from sender
  if ( radio.available( &receivedFrom ) )
  {
    if(receivedFrom == readingPipeSender){
      // Dump the payloads until we've gotten everything
      unsigned long message;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &message, sizeof(unsigned long) );
        
        // Spew it
        printf("Got payload %lu...",message);
        
        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(5);
      }
      
      // First, stop listening so we can talk
      radio.stopListening();
      radio.openWritingPipe(pipes[2]);
      // Send the final one back.
      radio.write( &message, sizeof(unsigned long) );
      printf("Relayed message to receiver.\n\r");
      // Now, resume listening so we catch the next packets.
      radio.startListening();  
    }else if(receivedFrom == readingPipeReceiver){
    
      // Dump the payloads until we've gotten everything
      unsigned long message;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &message, sizeof(unsigned long) );
        
        // Spew it
        printf("Got payload %lu...",message);
        
        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(5);
      }
      
      // First, stop listening so we can talk
      radio.stopListening();
      radio.openWritingPipe(pipes[1]);
  
      // Send the final one back.
      radio.write( &message, sizeof(unsigned long) );
      printf("Relayed message to sender.\n\r");
      // Now, resume listening so we catch the next packets.
      radio.startListening();   
    } 
  }
}
