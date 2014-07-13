#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);
const int audioPin = 1;
const uint64_t pipes[1] = { 0xdeadbeefa1LL };

int32_t x[4] = {100,200,300,400};
int32_t y[4] = {20,0,0,20};

uint8_t radio_received = 0;    //Received index of beacon
unsigned long radio_stopped;   //t0
unsigned long audio_started;   //t1
uint32_t distance;             //(t1-t0)*speed_of_sound
unsigned long speed_of_sound = 34421;//344.21 m/s = 34421 / 1000000 cm/us
int offset = 7;                //7 cm

uint32_t distances[4];         //calculated distances
int32_t x_crds[8];      	   //calculated coords
int32_t y_crds[8];      	   //calculated coords

int counter = 0;
int count   = 0;
boolean bereken = false;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
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
      if(distance< 700){//No false readings
        if(distance >= 0) {
          distances[radio_received] = distance;
        }
        else {
          distances[radio_received] = 0;
        }
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
  long r1 = distances[i];
  long r2 = distances[j];
  long r3 = distances[k];
  long x1 = x[i];
  long x2 = x[j];
  long x3 = x[k];
  long y1 = y[i];
  long y2 = y[j];
  long y3 = y[k];

  long c1 = (((r1*r1)-(r3*r3))-((x1*x1)-(x3*x3))-((y1*y1)-(y3*y3)))/2;
  long c2 = (((r2*r2)-(r3*r3))-((x2*x2)-(x3*x3))-((y2*y2)-(y3*y3)))/2;
  
  long n =(-((y3-y1)*10000)/(x3-x1)); 
  long m = ((c1*10000)/(x3-x1));

  long y = ((c2*10000) - m*(x3-x2))/(n*(x3-x2) + (10000*(y3-y2)));
  long x = ((n*y)+m)/10000; 
  
  if(!bereken){
    x_crds[counter] = x;
    y_crds[counter] = y;
  }else{
    x_crds[counter+4] = x;
    y_crds[counter+4] = y;
  }
}

void printGemiddelde(){
   int x_gemiddelde =  (x_crds[0]+x_crds[1]+x_crds[2]+x_crds[3]+x_crds[4]+x_crds[5]+x_crds[6]+x_crds[7])/8;
   int y_gemiddelde =  (y_crds[0]+y_crds[1]+y_crds[2]+y_crds[3]+y_crds[4]+y_crds[5]+y_crds[6]+y_crds[7])/8;
   printf("De gemiddelde x:%i  en y: %i waarden \n\r", x_gemiddelde, y_gemiddelde);
}
