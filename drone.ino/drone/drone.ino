#include <stdlib.h>
#include <Wire.h> 
#include <SPI.h>
#include <MPU6050_tockn.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <string.h>
#include <MQ135.h>
#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <math.h>
#include <Servo.h>

//màu mè thì thêm thư viện adafruit neo vào
//int x = 255, y = random(0,255), z = 0;      //tạo màu ngẫu nhiên cho led rgb ring

int a2212a, a2212b, a2212c, a2212d;

const byte address[5] = "3007"; // địa chỉ nào cx đc, miễn là 2 cái trùng nhau

bool tof = true, beginorend = false;
float xb, yb, xe, ye, pi = 3.14159265, time;
float xm, ym, xmb, ymb;
int x, y, loopsv = 0, tem = 0;
int distance, automode, num, ppm, high = 1000;
unsigned long timemillis, timempu, timesv;

Servo sv, esca, escb, escc, escd;
MQ135 gassensor = MQ135(6);
RF24 radio(9, 10);
MPU6050 degree(Wire);
//SoftwareSerial gps1(0,1);
//TinyGPSPlus gps;

void setup() { //setup
  Wire.begin();
  degree.begin();
  degree.calcGyroOffsets(true);

  //gps1.begin(9600);
  
  sv.attach(8);

  esca.attach(2);
  escb.attach(3);
  escc.attach(4);
  escd.attach(5);
  esca.writeMicroseconds(1000);
  escb.writeMicroseconds(1000);
  escc.writeMicroseconds(0);
  escd.writeMicroseconds(1000);
  
  radio.begin();
  if (!radio.begin()) 
  {
    while (1) {
    Serial.println("Module không khởi động được...!!");}
  }
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);                    //đặt cường độ phát sóng là max
  radio.setChannel(80);
  radio.setDataRate(RF24_250KBPS);
  radio.startListening();

  xmb = degree.getAngleX();   //xem lại cái của nợ này
  ymb = degree.getAngleY();
  xm = xmb;
  ym = ymb;

  /*while (gps1.available() <= 0){     //đây nữa, xem lại
     if (gps.encode(gps1.read())){
        if (gps.location.isValid()){
         xb = gps.location.lat();
         yb = gps.location.lng();
        }
     }
  }*/
  Serial.begin(9600);
  delay(10000);
  Serial.println("ok");
  timemillis = millis();
  timempu = timemillis;
  /*sv.write(180);
  delay(500);
  sv.write(0);*/
}

void mpu(){    //cân bằng
  degree.update();
  if (abs(degree.getAngleX() - xm) > 1){
    xm = degree.getAngleX();
  }
  if (abs(degree.getAngleY() - ym) > 1){
    ym = degree.getAngleY();
  }
  if  (xm - xmb >= 7){
    a2212a -= 10;
    a2212b += 10;
    a2212c += 10;
    a2212d -= 10;
  }
  if(xmb - xm >= 7){
    a2212a += 10;
    a2212b -= 10;
    a2212c -= 10;
    a2212d += 10;
  }
  if (ym - ymb >= 7){
    a2212a += 10;
    a2212b += 10;
    a2212c -= 10;
    a2212d -= 10;
  }
  if(ymb - ym >= 7){
    a2212a -= 10;
    a2212b -= 10;
    a2212c += 10;
    a2212d += 10;
  }
}

void loop(){
  if (radio.available()){ // nhận tín hiệu
    radio.read(&num, sizeof(num));
  }else{
    num = -2;
  }

  if (num != -2){
    if (num != -1){  //ko tự động
      /*if (tof == true){    //lấy vị trí gốc
        if (gps1.available() > 0){
          if (gps.encode(gps1.read())){
            if (gps.location.isValid()){
              xb = gps.location.lat();
              yb = gps.location.lng();
            }
          }
        }
      }
      tof = false;*/
      if (num > 99){  //nhận tín hiệu
        high = num * 10;
        a2212a = high;
        a2212b = a2212a;  
        a2212c = a2212a;
        a2212d = a2212a;
      }else if (num == 100){  //đứng im
        a2212b = high;
        a2212c = high;
        a2212d = high;
      }else if (num == 10){  //tiến
        a2212a = high - 10;
        a2212b = high - 10;
        a2212c = high + 10;
        a2212d = high + 10;
      }else if (num == 20){   // lùi
        a2212a = high + 10;
        a2212b = high + 10;
        a2212c = high - 10;
        a2212d = high - 10;
      }else if (num == 30){   //trái
        a2212a = high - 10;
        a2212b = high + 10;
        a2212c = high + 10;
        a2212d = high - 10;
      }else if (num == 40){   //phải
        a2212a = high + 10;
        a2212b = high - 10;
        a2212c = high - 10;
        a2212d = high + 10;
      }else if (num == 3){   //xoay phải
        a2212a = high + 20;
        a2212b = high - 20;
        a2212c = high + 20;
        a2212d = high - 20;
      }else if(num == 4){     //xoay trái
        a2212a = high - 20;
        a2212b = high + 20;
        a2212c = high - 20;
        a2212d = high + 20;
      }

      if (millis() - timempu >= 500){mpu();}
      
      esca.writeMicroseconds(a2212a);
      escb.writeMicroseconds(a2212b);
      escc.writeMicroseconds(map(a2212c, 1000, 2000, 978, 2002));
      escd.writeMicroseconds(a2212d);

      //ppm = gassensor.getPPM();

      /*if (num == 1){     //cái của nợ này là gắp bóng
        if (millis() - timesv >= 2000){
          if (loopsv == 0){
            sv.write(180);
            loopsv = 1;
          }else{
            sv.write(0);
            loopsv = 0;
          }
        }
      }*/
      
    }/*else{     // code tự động đỡ pk điều khiển  
      if (tof = false){
        if (gps1.available() > 0){
          if (gps.encode(gps1.read())){
            if (gps.location.isValid()){
              xe = gps.location.lat();
              ye = gps.location.lng();
              beginorend = true;
            }
          }
        }
      }else{
        if (gps1.available() > 0){
          if (gps.encode(gps1.read())){
            if (gps.location.isValid()){
              x = gps.location.lat();
              y = gps.location.lng();
              if (beginorend == false){
                if (xe > x){
                  if (ye > y){
                    if (atan((ye - y)/(xe - x)) * 180 / pi > 45){
                      automode = 1;
                    }else{
                      automode = 3;
                    }
                  }else{
                    if (atan((y - ye)/(xe - x) * 180 / pi > 45)){
                      automode = 2;
                    }else{
                      automode = 3;
                    }
                  }
                }else{
                  if (ye > y){
                    if (atan((ye - y)/(x - xe)) * 180 / pi > 45){
                      automode = 1;
                    }else{
                      automode = 4;
                    }
                  }else{
                    if (atan((y - ye)/(x - xe)) * 180 / pi > 45){
                      automode = 2;
                    }else{
                      automode = 4;
                    }
                  }
                }
              }else{
                if (xb > x){
                  if (yb > y){
                    if (atan((yb - y)/(xb - x)) * 180 / pi > 45){
                      automode = 1;
                    }else{
                      automode = 3;
                    }
                  }else{
                    if (atan((y - yb)/(xb - x) * 180 / pi > 45)){
                      automode = 2;
                    }else{
                      automode = 3;
                    }
                  }
                }else{
                  if (yb > y){
                    if (atan((yb - y)/(x - xb)) * 180 / pi > 45){
                      automode = 1;
                    }else{
                      automode = 4;
                    }
                  }else{
                    if (atan((y - yb)/(x - xb)) * 180 / pi > 45){
                      automode = 2;
                    }else{
                      automode = 4;
                    }
                  }
                }
              }
            }
          }
        }
        if (beginorend == true){
          if (xb - x <= 10 && yb - y <= 10){
            beginorend = false;
            time = millis();
            while (millis() <= time + 10000){
              if (a2212a > 1050){
                a2212a -= 5;
              }
              a2212b = a2212a;
              a2212b = a2212a;
              a2212b = a2212a;
              for (int i = 0; i <= 10; i++){  
                esca.writeMicroseconds(a2212a);
                escb.writeMicroseconds(a2212b);
                escc.writeMicroseconds(map(a2212c, 1000, 2000, 978, 2002));
                escd.writeMicroseconds(a2212d);
              }
            }
            time = millis();
            while (millis() <= time + 10000){
              if (a2212a < 1950){
                a2212a += 5;
              }
              a2212b = a2212a;
              a2212b = a2212a;
              a2212b = a2212a;
              esca.writeMicroseconds(a2212a);
              escb.writeMicroseconds(a2212b);
              escc.writeMicroseconds(map(a2212c, 1000, 2000, 978, 2002));
              escd.writeMicroseconds(a2212d);
            }
          }
        }else{
          if (xe - x <= 10 && ye - y <= 10){
            beginorend = true;
            if (loopsv == 0){
              sv.write(180);
              loopsv = 1;
            }else{
              sv.write(0);
              loopsv = 0;
            }
          }
        }
        if (automode == 1){
          a2212a = high - 10;
          a2212b = high - 10;
          a2212c = high + 10;
          a2212d = high + 10;
        }else if (automode == 2){
          a2212a = high + 10;
          a2212b = high + 10;
          a2212c = high - 10;
          a2212d = high - 10;
        }else if (automode == 3){
          a2212a = high - 10;
          a2212b = high + 10;
          a2212c = high + 10;
          a2212d = high - 10;
        }else if (automode == 4){
          a2212a = high + 10;
          a2212b = high - 10;
          a2212c = high - 10;
          a2212d = high + 10;
        }
        if (millis() - timempu >= 1000){
          mpu();
        }
        esca.writeMicroseconds(a2212a);
        escb.writeMicroseconds(a2212b);
        escc.writeMicroseconds(map(a2212c, 1000, 2000, 978, 2002));
        escd.writeMicroseconds(a2212d);
      }
    }*/
  }else{    //lúc mất tín hiệu
    a2212b = a2212a;
    a2212c = a2212a;
    a2212d = a2212a;
    if (millis() - timempu >= 1000){
      mpu();
    }
    esca.writeMicroseconds(a2212a);
    escb.writeMicroseconds(a2212b);
    escc.writeMicroseconds(map(a2212c, 1000, 2000, 978, 2002));
    escd.writeMicroseconds(a2212d);
  }
}