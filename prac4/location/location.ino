#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
const int audioPin = 1;
const uint64_t pipes[1] = { 0xdeadbeefa1LL };

int x[4] = {0,0,0,0};
int y[4] = {0,0,0,0};

uint8_t radio_received = 0;    //Received index of beacon
unsigned long radio_stopped;   //t0
unsigned long audio_started;   //t1
uint32_t distance;             //(t1-t0)*speed_of_sound
unsigned long speed_of_sound = 34421;//344.21 m/s = 34421 / 1000000 cm/us
int offset = 7;                //7 cm
uint32_t distances[4];

int count = 0;

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
      if(analogRead(audioPin)>0){//radio signal received
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
      distance = distance -offset;
      if(distance< 1000){//No false readings
        distances[radio_received] = distance;
      }
    }  
    if(count == 3){
     for(int i=0;i<4;i++){
       printf("%i : %ld cm", i, distances[i]);
     } 
     printf("\r\n");
     //TODO: x/y coords 
    }
    count = (count +1) % 4;
  }
  calculateXY(0,0,0);
}

void calculateXY(int i, int j, int k){
  uint32_t r1 = 3;//distances[i];
  uint32_t r2 = 2;//distances[j];
  uint32_t r3 = 3;//distances[k];
  
  int x1 = 2;//x[i];
  int x2 = 5;//x[j];
  int x3 = 8;//x[k];
  
  int y1 = 1;//y[i];
  int y2 = 4;//y[j];
  int y3 = 2;//y[k];
  
  long c1 = 32;//((r1^2-r3^2)-(x1^2-x3^2)-(y1^2-y3^2))/2;
  long c2 = 11;//((r2^2-r2^2)-(x2^2-x3^2)-(y2^2-y3^2))/2;
  
  long n =-(y3-y1)/(x3-x1);
  long m = c1/(x3-x1);
  
  long y = (c2 - m*(x3-x2))/(n*(x3-x2) + (y3-y2));
  long x = n*y+m; 
  
  printf("[%ul, %ul]", x, y);
}
