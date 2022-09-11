#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "Arduino.h"
#include <AverageValue.h>

#define RELAY_ON 0
#define RELAY_OFF 1
#define RELAY_1 2

LiquidCrystal_I2C lcd(0x27,20,4);

int MQ1=A0;
int Ph1=A1;
int Tbd1=A2;
int MQ2=A3;
int Ph2=A4;
int Tbd2=A6;

//MQ
int Rload = 20000;
float rO=12000;
double ppm=413.93;
float a = 110.7432567;
float b = -2.856935538;
float minppm=0;
float maxppm=0;
const long MAX_VALUES_NUM = 10;
AverageValue<long> averageValue(MAX_VALUES_NUM);
AverageValue1<long> averageValue1(MAX_VALUES_NUM);

//Turbidity
unsigned long start_times[300];
unsigned long stop_times[300];
unsigned long values[300];

const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);

//PH
int buf[10];
float ph (float voltage){
  return 7 * ((2.5 - voltage) / 0.18);  
}

void setup(){
    lcd.init();
    lcd.backlight();
    pinMode(MQ1, INPUT);
    pinMode(MQ2, INPUT);
    pinMode(Ph1, INPUT);
    pinMode(Ph2, INPUT);

  pinMode(RELAY_1, OUTPUT);

  digitalWrite(RELAY_1, RELAY_OFF);
    
    Serial.begin(9600);
    
     //min[Rs/Ro]=(max[ppm]/a)^(1/b)
       minppm=pow((1000/110.7432567),1/-2.856935538);
      //max[Rs/Ro]=(min[ppm]/a)^(1/b)
       maxppm=pow((10/110.7432567),1/-2.856935538);
    ;
    ADCSRA &= ~PS_128;
    ADCSRA |= PS_128;

}

void loop(){
  mq1();
  mq2();
  ph1();
  ph2();
  tbd1();
  tbd2();
  
  lcd.setCursor(0,0);
  lcd.print("Filterisasi");\
  
  lcd.setCursor(0,1);
  lcd.print("MQ1:");
  lcd.setCursor(5,1);
  lcd.print(averageValue.average());
  lcd.setCursor(0,2);
  lcd.print("Ph1:");
  lcd.setCursor(5,2);
  lcd.print(phValue);
  lcd.setCursor(0,3);
  lcd.print("Tb1:");
  lcd.setCursor(5,3);
  lcd.print(ntu);

  lcd.setCursor(9,1);
  lcd.print("MQ2:");
  lcd.setCursor(5,1);
  lcd.print(averageValue1.average());
  lcd.setCursor(9,2);
  lcd.print("Ph2:");
  lcd.setCursor(9,2);
  lcd.print(phValue1);
  lcd.setCursor(9,3);
  lcd.print("Tb2:");
  lcd.setCursor(9,3);
  lcd.print(ntu1);

  if(phValue1 <+ 6 && phValue1 +> 8){
    relay_on();
    if(ntu1 > 200){
      relay_on();
      }
     else {
      relay_off();
      } 
   }
   else{
     relay_off();
   }
}

void mq1(){
int adcRaw = analogRead(MQ1);
double rS = ((1024.0 * Rload) / adcRaw) - Rload;
   
float rSrO= rS/rO;
if(rSrO < maxppm && rSrO > minppm) 
  {
    float ppm = a * pow((float)rS / (float)rO, b);
    averageValue.push(ppm);
  }
}

void mq2(){
int adcRaw = analogRead(MQ2);
double rS = ((1024.0 * Rload) / adcRaw) - Rload;
   
float rSrO= rS/rO;
if(rSrO < maxppm && rSrO > minppm) 
  {
    float ppm = a * pow((float)rS / (float)rO, b);
    averageValue1.push(ppm);
  }
}

void ph1(){
for(int i=0;i<10;i++){  
  buf[i]=analogRead(Ph1);
  delay(10);
  }
float avgValue=0;
for(int i=0;i<10;i++)
avgValue+=buf[i];

float phVol=(float)avgValue*5.0/1024/10;
float phValue = -6.21 * phVol +   22.23;
}

void ph2(){
for(int i=0;i<10;i++){  
  buf[i]=analogRead(Ph2);
  delay(10);
  }
float avgValue=0;
for(int i=0;i<10;i++)
avgValue+=buf[i];

float phVol=(float)avgValue*5.0/1024/10;
float phValue1 = -6.5 * phVol +   31.10;
}

void tbd1(){
  unsigned int i;
  unsigned int z;
  z = 0;
   
  // capture the values to memory
  for(i=0;i<300;i++) {
    start_times[i] = micros();
    values[i] = analogRead(Tbd1);             
 
  if (values[i] >= z) {
  z = values[i]; 
  }
 
    stop_times[i] = micros();


  }
 
  float ntu = ((z - 694.7)/-0.3803)+69;
}

void tbd2(){
  unsigned int i;
  unsigned int z;
  z = 0;
   
  // capture the values to memory
  for(i=0;i<300;i++) {
    start_times[i] = micros();
    values[i] = analogRead(Tbd2);             
 
  if (values[i] >= z) {
  z = values[i]; 
  }
 
    stop_times[i] = micros();


  }
 
  float ntu1 = (z - 717.71)/-0.3327;
}

void relay_on(){
  digitalWrite(RELAY_1, RELAY_ON);
}

void relay_off(){
  digitalWrite(RELAY_1, RELAY_OFF);
}
