#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

unsigned long received = 0;
unsigned long sent = -1;
unsigned long initVal = -1;
bool started = false;
bool stopped = false;


void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  received = 0;
  printf("\n\rRF24/examples/pingpair/\n\r");

  //
  // Setup and configure rf radio
  //
  radio.begin();
  radio.setChannel(125);
  radio.setRetries(0,0);
  radio.setPayloadSize(255);
  radio.setDataRate(RF24_2MBPS);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
}


void loop(void)
{
  // if there is data ready
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &sent, sizeof(unsigned long) );
      stopped = received >= 1000;
      // Spew it   
      if(started && !stopped){
        received = received+1;
        printf("Got packet: %u", received);
        printf(" payload: %u", (sent-initVal));
        printf(" lost: %u\r\n", ((sent-initVal)-received));
      }else if(!started && sent != -1 ){
          started = true;
         initVal = sent; 
      }
	// Delay just a little bit to let the other unit
	// make the transition to receiver
	delay(20);
    }
    
    // First, stop listening so we can talk
    radio.stopListening();

    // Now, resume listening so we catch the next packets.
    radio.startListening();
  }
}
