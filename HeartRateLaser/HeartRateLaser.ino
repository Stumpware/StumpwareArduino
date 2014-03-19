/*
 LaserVectorPainter 
 P. Mark Anderson
 
 Sweep a servo with an attached laser 
 to draw vector geometry 
 on a surface.
 
 Version 1.0 2013/10/28 PMA
 */


#include <Servo.h>

#define MIN_SERVO_DELAY 95
#define ANGLE_RANGE 20
#define SERVO1 6
#define SERVO2 5

Servo servo1;
Servo servo2;

typedef struct {
  float x;
  float y;
} Point;

typedef enum {
  Mode0,
  Mode1
} Mode;

void drawLine(Point p1, Point p2);
void goToPoint(Point p);
uint8_t angleForPosition(float position);


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  servo1.attach(SERVO1);
  servo2.attach(SERVO2);
}

#define DURATION 1500.0

Point p1;
Mode mode;

void loop() {

  Point p1;  
  p1.x = 0.0;
  p1.y = 1.0;
  float elapsedTime, totalElapsedTime;
  uint16_t cnt1Start = millis();     // Mode switcher
  uint16_t cnt2Start = cnt1Start;    // Laser sweeper
  
  while (1) {
    uint16_t now = millis();
    uint16_t cnt1Elapsed = (now - cnt1Start);  
    uint16_t cnt2Elapsed = (now - cnt2Start);

#if 0    
    if (cnt1Elapsed > (DURATION * 1)) {
      switch (mode) {
        case Mode0:
          mode = Mode1;
          break;
        case Mode1:
          mode = Mode0;
          break;
      }
      
      cnt1Start = millis();
    }    
#endif
    
    if (mode == Mode0) {
//      Serial.print("cnt2Start: ");
//      Serial.print(cnt2Start);
//      Serial.print(", cnt2Elapsed: ");
//      Serial.println(cnt2Elapsed);

      p1.x = 1.0 - (cnt2Elapsed / DURATION);

      if (cnt2Elapsed > DURATION || p1.x > 1.0) {
        p1.x = 0.0;
        p1.y = 1.0;        
        cnt2Start = millis();
//        Serial.println("BACK");
      }
      else if (p1.x > 0.25 && p1.x < 0.35) {
        p1.y = 0.5;
      }
      else if (p1.x > 0.35 && p1.x < 0.4) {
        p1.y = 0.8;
      }
      else if (p1.x > 0.4 && p1.x < 0.5) {
        // Peak
        p1.y = 1.5;
      }
      else if (p1.x > 0.5 && p1.x < 0.6) {
        p1.y = 0.6;
      }
      else if (p1.x > 0.6 && p1.x < 0.7) {
        p1.y = 1.2;
      }
      else if (p1.x > 0.7) {
        p1.y = 0.0;
      }
      
    }
    else {
      p1.x = 0.0;
      p1.y = 0.5;
//      Serial.print("cnt1Start: ");
//      Serial.print(cnt1Start);
//      Serial.print(", cnt1Elapsed: ");
//      Serial.println(cnt1Elapsed);
    }

    goToPoint(p1);
  }
}


void drawLine(Point p1, Point p2) {
  goToPoint(p1);
  goToPoint(p2);
}

void goToPoint(Point p) {  
  int xAngle = angleForPosition(p.x);
  int yAngle = angleForPosition(p.y);
//  Serial.print(p.x);
//  Serial.print(": ");
//  Serial.println(angle);
  servo1.write(xAngle);
  servo2.write(yAngle);
  delay(MIN_SERVO_DELAY);
}

uint8_t angleForPosition(float position) {
  return (position * ANGLE_RANGE) + (45 - ANGLE_RANGE / 2);
}



