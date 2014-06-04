#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
const int audioPin = 1;
const uint64_t pipes[1] = { 0xdeadbeefa1LL };

uint8_t radio_received = 0;
unsigned long radio_stopped;
unsigned long audio_started;
uint32_t distance;
unsigned long speed_of_sound = 34421;//344.21 m/s = 34421 / 1000000 cm/us
int offset = 7;//7 cm
uint32_t distances[4];

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  //
  // Setup and configure rf radio
  //
  radio.begin();
  radio.setChannel(76);
  radio.setRetries(0,0);
  radio.setPayloadSize(1);
  radio.setDataRate(RF24_2MBPS);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.setAutoAck(false);
  radio.printDetails();
}

void loop(void)
{
  if ( radio.available() )
  {
    bool done = false;
    while (!done)
    {
      done = radio.read( &radio_received, sizeof(uint8_t) );
    }
    bool received = false;
    bool waiting = true;
    radio_stopped = micros();
    while(waiting){
      if(analogRead(audioPin)!=0){//radio signal received
        audio_started = micros();
        received = true;
        waiting = false;
      }
      else if(micros()-radio_stopped > 75000){//nothing received after radio
        waiting = false;
      }
    }
    
    if(received){
      distance = (audio_started-radio_stopped)*speed_of_sound/1000000;
      distances[radio_received] = distance-offset;
      if(radio_received == 3){
         for(int i=0;i<4;i++){
           printf("%i : %ld cm", i, distances[i]);
         } 
           printf("\r\n");
       //TODO: x/y coords 
       }
    }  
  }
}
