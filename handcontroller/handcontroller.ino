#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <LiquidCrystal_I2C.h> 
#include <string.h>
#include <SoftwareSerial.h>

RF24 radio(9, 10);
//LiquidCrystal_I2C lcd(0x27, 16, 2); 
SoftwareSerial bluetooth(2,3);

const byte address[4] = "3007";

long timer = 0;
unsigned long nowtime = 0;
int g = 0, b = 1, l = 4, r = 5, deathpress = 6, gblr = 0, high = 1015, turn, autof = 7;
int ppm, heat, sus = -1, press;
int x, y, in4;
bool autofly = false;
char gtri, gtri1;

void setup() {
  Wire.begin();
  pinMode(g, INPUT);
  pinMode(b, INPUT);
  pinMode(l, INPUT);
  pinMode(r, INPUT);
  pinMode(deathpress, INPUT);
  pinMode(autof, INPUT);
  
/*  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.home();*/

  bluetooth.begin(9600);
  Serial.begin(9600);

  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setChannel(80);
  radio.setDataRate(RF24_250KBPS);
}

void loop() {
  //lcd.home();
  radio.stopListening();
  if (autofly == false){
    if (digitalRead(autof) == LOW){
      autofly = true;
    }
    if (digitalRead(g) == LOW){
      gblr = 1;
    }else if (digitalRead(b) == LOW){
      gblr = 2;
    }else if (digitalRead(l) == LOW){
      gblr = 3;
    }else if (digitalRead(r) == LOW){
      gblr = 4;
    }else{
      gblr = 10;
    }
    
    if (digitalRead(deathpress) == LOW){
      press = 1;
    }else{
      press = 0;
    } 
    high = map(analogRead(A0),0,1023,101,199);
    if (high <= 101){
      high = 102;
    }else if (high >= 199){
      high = 198;
    }
    if (analogRead(A1) <= 212){
      turn = 3;
    }else if (analogRead(A1) >= 812){
      turn = 5;
    }else{
      turn = 4;
    }
    if (bluetooth.available() > 0){
      gtri = bluetooth.read();
        if (gtri == "g"){
          gblr = 1;
        }else if(gtri == "b"){
          gblr = 2;
        }else if(gtri == "l"){
          gblr = 3;
        }else if(gtri == "r"){
          gblr = 4;
        }
                
        if(gtri == "u"){
          high += 1;
        }else if(gtri == "d"){
          high -= 1;
        }else if(gtri == "R"){
          turn = 4;
        }else if(gtri == "L"){
          turn = 3;
        }
    }
    
    Serial.println(gtri);
    gblr = gblr * 10;
    radio.write(&gblr, sizeof(gblr));
    radio.write(&turn, sizeof(turn));
    radio.write(&press, sizeof(press));
    radio.write(&high, sizeof(high));
  }else{
    radio.write(&sus, sizeof(sus));
    if (digitalRead(autof) == LOW){
      autofly = false;
    }
  }
  delay(50);
} 