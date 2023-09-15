/////////////////////////// encoder interrupt //////////////////////////
#include "src/PinChangeInt.h"

#define ENCODER_A1      2                       // Quadrature encoder A pin
#define ENCODER_B1      3                       // Quadrature encoder B pin

volatile long g_encoderpos = 0;

void InterruptSetup(void){
  pinMode(ENCODER_A1, INPUT_PULLUP);                  // quadrature encoder input A
  pinMode(ENCODER_B1, INPUT_PULLUP);                  // quadrature encoder input B
  attachInterrupt(0, EncoderHandler, FALLING);               // update encoder position
  TCCR1B = TCCR1B & 0b11111000 | 1;                   // To prevent Motor Noise  
}

void EncoderHandler(){                                              // pulse and direction, direct port reading to save cycles  
  //Serial.println("interrupt");
  if(digitalRead(ENCODER_B1)==HIGH)    g_encoderpos--;             // if(digitalRead(ENCODER_B1)==HIGH)   count --;
  else                                 g_encoderpos++;             // if(digitalRead(ENCODER_B1)==LOW)   count ++;
}

/////////////////////////// I2C 통신  //////////////////////////

#include <Wire.h>

// receive data
void ReceiveEvent(int byte_count){
  unsigned  char received_data[7];
  if(byte_count ==8){
    for(int i=0;i<8;i++){
      received_data[i]=Wire.read();
      Serial.print(a[i]);
      Serial.print(" ");
    }
  Serial.println("\nreceive");
  }
}

// send data
void RequestEvent(){
  unsigned char send_data[8] = {0,};
  int temp;
  temp = 1*10;
  send_data[0]='#';
  send_data[1]= (temp&0xff00)>>8;
  send_data[2]= (temp&0x00ff); 
  send_data[3]= (g_encoderpos&0xff000000)>>24;    // encoder MSB 8bit
  send_data[4]= (g_encoderpos&0x00ff0000)>>16;
  send_data[5]= (g_encoderpos&0x0000ff00)>>8;
  send_data[6]= (g_encoderpos&0x000000ff);        // encoder LSB 8bit
  send_data[7]='*'; 
  
  Wire.write(send_data,8); // respond 
  Serial.println("send");
}

/////////////////////////// setup //////////////////////////

void setup()
{
  InterruptSetup();
  Wire.begin(0x05);    // I2C bus  #5
  Wire.onRequest(RequestEvent); // register events
  Wire.onReceive(ReceiveEvent);
  Serial.begin(115200);
  delay(2000);
}

/////////////////////////// loop //////////////////////////

void loop()
{
  Serial.println(g_encoderpos);
  delay(100);
}
