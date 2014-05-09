#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
int messages_sent = 0;

const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  printf("\n\rRF24/examples/pingpair/\n\r");
  //
  // Setup and configure rf radio
  //
  radio.begin();
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_2MBPS);
  radio.setRetries(0,0);
  radio.setPayloadSize(255);
  radio.setChannel(125);
  radio.openWritingPipe(pipes[0]);
  radio.startListening();
  radio.printDetails();
}
void loop(void){
    // First, stop listening so we can talk.
    radio.stopListening();

    printf("Now sending...");
    bool ok = radio.write( &messages_sent, sizeof(unsigned int) );
    messages_sent++;
    printf("number of messages sent: %u\n\r",messages_sent);
   
    // Now, continue listening
    radio.startListening();
	
	// Wait before sending next message
    delay(25);
}
