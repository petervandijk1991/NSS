#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
const int audioPin = 1;
const uint64_t pipes[1] = { 0xdeadbeefa1LL };

int32_t x[4] = {0,72,294,372};
int32_t y[4] = {75,0,0,136};

uint8_t radio_received = 0;    //Received index of beacon
unsigned long radio_stopped;   //t0
unsigned long audio_started;   //t1
uint32_t distance;             //(t1-t0)*speed_of_sound
unsigned long speed_of_sound = 34421;//344.21 m/s = 34421 / 1000000 cm/us
int offset = 7;                //7 cm
uint32_t distances[4];
int32_t x_coordinates[4];
int32_t y_coordinates[4];

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
       printf("%i : %ld cm ", i, distances[i]);
     } 
     printf("\n\r");
     calculateXY(0,1,2); 
    }
    count = (count +1) % 4;
  }
  
}

void calculateXY(int i, int j, int k){
  uint32_t r1 = distances[i];
  uint32_t r2 = distances[j];
  uint32_t r3 = distances[k];
  
  int32_t x1 = x[i];
  int32_t x2 = x[j];
  int32_t x3 = x[k];
  printf("De waarde van x3 = %d \n\r", x3);
  
  
  int32_t y1 = y[i];
  int32_t y2 = y[j];
  int32_t y3 = y[k];
  
  uint32_t r1_r1 = r1*r1;
  uint32_t r2_r2 = r2*r2;
  uint32_t r3_r3 = r3*r3;
  uint32_t x1_x1 = x1*x1;
  uint32_t x2_x2 = x2*x2;
  uint32_t x3_x3 = x3*x3;
  uint32_t y1_y1 = y1*y1;
  uint32_t y2_y2 = y2*y2;
  uint32_t y3_y3 = y3*y3;

  //Beide c1 en c2 berekeningen gaan goed!
  long c1 = (((r1_r1)-(r3_r3))-((x1_x1)-(x3_x3))-((y1_y1)-(y3_y3)))/2;
   printf("De waarde van c1 = ");
  Serial.print(c1);
  printf("\n\r");
  long c2 = (((r2_r2)-(r3_r3))-((x2_x2)-(x3_x3))-((y2_y2)-(y3_y3)))/2;
  printf("De waarde van c2 = ");
  Serial.print(c2);
  printf("\n\r");
  
  
  long n =(-((y3-y1)*10000)/(x3-x1)); // 0.2551020
  printf("De waarde van n = ");
  Serial.print(n);
  printf("\n\r");
  
  long m = ((c1*10000)/(x3-x1));//32/6
  printf("De waarde van m = ");
  Serial.print(m);
  printf("\n\r");

  
  long y = ((c2*10000) - m*(x3-x2))/(n*(x3-x2) + (10000*(y3-y2)));
  
  long c2_10000 = c2*10000;
  printf("De waarde van c2 * 10000 = ");
  Serial.print((c2_10000));
  printf("\n\r");
  
  long m_x3_x2 = m*(x3-x2);
  printf("De waarde van m*(x3-x2) = ");
  Serial.print(m_x3_x2);
  printf("\n\r");
  
  long n_x3_x2 = n*(x3-x2);
  printf("De waarde van n*(x3-x2) ");
  Serial.print(n_x3_x2);
  printf("\n\r");
  
  long y3_y2 = 10000*(y3-y2);
  printf("De waarde van 10000*(y3-y2) = ");
  Serial.print(y3_y2);
  printf("\n\r");
  printf("De waarde van y = ");
  Serial.print(y);
  printf("\n\r");
  
  
  long x = ((n*y)+m)/10000; 
  printf("De waarde van x = ");
  Serial.print(x);
  printf("\n\r");
}
