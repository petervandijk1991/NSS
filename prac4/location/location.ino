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

uint32_t distances[4];         //calculated distances
int32_t x_coordinates[8];      //calculated coords
int32_t y_coordinates[8];      //calculated coords

int counter = 0;
boolean bereken = false;
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
     counter = 0;
     while(counter < 4){
       calculateXY((counter%4) , ((counter+1)%4), ((counter+2)%4));
       counter++;
     }
     if(bereken){
       printGemiddelde();
     }
     bereken = !bereken; 
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
  long c2 = (((r2_r2)-(r3_r3))-((x2_x2)-(x3_x3))-((y2_y2)-(y3_y3)))/2;
  
  long n =(-((y3-y1)*10000)/(x3-x1)); // 0.2551020
  long m = ((c1*10000)/(x3-x1));//32/6

  long y = ((c2*10000) - m*(x3-x2))/(n*(x3-x2) + (10000*(y3-y2)));
  long x = ((n*y)+m)/10000; 
  
  printf("[");
  Serial.print(y);
  printf(",");
  Serial.print(x);
  printf("]\n\r");
  
  if(!bereken){
    x_coordinates[counter] = x;
    y_coordinates[counter] = y;
  }else{
    x_coordinates[counter+4] = x;
    y_coordinates[counter+4] = y;
  }
}

void printGemiddelde(){
   int x_gemiddelde =  (x_coordinates[0]+x_coordinates[1]+x_coordinates[2]+x_coordinates[3]+x_coordinates[4]+x_coordinates[5]+x_coordinates[6]+x_coordinates[7])/8;
   int y_gemiddelde =  (y_coordinates[0]+y_coordinates[1]+y_coordinates[2]+y_coordinates[3]+y_coordinates[4]+y_coordinates[5]+y_coordinates[6]+y_coordinates[7])/8;
   printf("De gemiddelde x:%i  en y: %i waarden \n\r", x_gemiddelde, y_gemiddelde);
}
