#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);

const int role_pin = 7;
int received = 0;

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role;

void setup(void)
{
 
  pinMode(role_pin, INPUT);
  digitalWrite(role_pin,HIGH);
  delay(20); // Just to get a solid reading on the role pin

  // read the address pin, establish our role
  if ( ! digitalRead(role_pin) )
    role = role_ping_out;
  else
    role = role_pong_back;

  Serial.begin(57600);
  printf_begin();
  received = 0;
  printf("\n\rRF24/examples/pingpair/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);

  //
  // Setup and configure rf radio
  //

  radio.begin();
  radio.setChannel(42);
  radio.setRetries(0,0);
  radio.setPayloadSize(255);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
  
}


bool started = false;

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
     unsigned long sent;
      done = radio.read( &sent, sizeof(unsigned long) );
      // Spew it   
       if(sent == 1){
          started = true; 
        }
      if(started){
        received = received+1;
        printf("Got packet: %u", received);
        printf(" payload: %u\r\n", sent);
      }else{    
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
// vim:cin:ai:sts=2 sw=2 ft=cpp
