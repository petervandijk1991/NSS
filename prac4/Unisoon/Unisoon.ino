#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

RF24 radio(3, 9);

const uint64_t pipes[1] = { 0xdeadbeefa1LL }; //De letter die op l en 1 lijkt is een één

unsigned long our_time;
unsigned long time0;
unsigned long time1;
unsigned long time2;
unsigned long time3;

//variabelen voor audio ontvangen
int distance0 = 0;
int distance1 = 0;
int distance2 = 0;
int distance3 = 0;


uint8_t received = 0;

void setup(void)
{
  Serial.begin(57600);
  printf_begin();
  
  //
  // Setup and configure rf radio
  //
  radio.begin();
  radio.setChannel(76);
  radio.setPayloadSize(1);
  radio.setDataRate(RF24_2MBPS);
  radio.openReadingPipe(1,pipes[0]);
  radio.setAutoAck(false);
  radio.startListening();
  radio.printDetails();
  
}
int audio_received = 0;
int aantal_ontvangen = 0;
void loop(void)
{
  
  //Luisteren naar de radio totdat wat wordt ontvangen STAP 1
  if ( radio.available() )
 {
   printf("We ontvangen iets op de radio \n\r");
   bool done = false;
   while (!done)
  {
    done = radio.read( &received, sizeof(uint8_t) );
     our_time = micros();
   }
     printf("payload is: %i \n\r", received);
  }
  //Audio ontvangen
  if(aantal_ontvangen < 4){
    if((aantal_ontvangen % 4) == 0 ) {
       audio_received = analogRead(A1);
       printf("De ontvangen waarde van r0: %i \n\r", audio_received);
      time0 = micros() - our_time; 
      distance0 = time0 * 343;
      printf("distance 0 = %i \n\r", distance0);
      aantal_ontvangen++;
    }
    else if((aantal_ontvangen % 4) == 1 ){
        audio_received = analogRead(A1);
       printf("De ontvangen waarde van r1: %i \n\r", audio_received);
      time1 = micros() - our_time;
      distance1 = time1 * 343;
      printf("distance 1 = %i \n\r", distance1);
     aantal_ontvangen++; 
    }
    else if((aantal_ontvangen % 4) == 2 ){
        audio_received = analogRead(A1);
       printf("De ontvangen waarde van r2: %i \n\r", audio_received);
      time2 = micros() - our_time; 
      distance2 = time2 * 343;
      printf("distance 2 = %i \n\r", distance2);
      aantal_ontvangen++;
    }
    else if((aantal_ontvangen % 4) == 3 ){
        audio_received = analogRead(A1);
       printf("De ontvangen waarde van r3: %i \n\r", audio_received);
      time3 = micros() - our_time;
     distance3 = time3 * 343; 
     printf("distance 3 = %i \n\r", distance3);
      aantal_ontvangen++;
    }
  }
  else{
    printf("klaar \n\r" );  
  }
  
  
   
  
  
}
