/*
 LaserVectorPainter 
 P. Mark Anderson
 
 Sweep a servo with an attached laser 
 to draw vector geometry 
 on a surface.
 
 Version 1.0 2013/10/28 PMA
 */


#include <Servo.h>

#define MIN_SERVO_DELAY 110

Servo servo;  

typedef struct {
  float x;
  float y;
} Point;

void drawLine(Point p1, Point p2);
void goToPoint(Point p);
uint8_t angleForPosition(float position);


void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  servo.attach(9);
}

void loop() {
  uint16_t startTime = millis();

  Point p1, p2;
  
#if 0
  int r = 50.0;  // Take r up to 100 for full range.
  p1.x = 0.45; //(0 + random(r)) / 100.0;
  p1.y = 0;
  p2.x = 0.55; //(r/2 + random(r)) / 100.0;
  p2.y = 0;

  while (millis() - startTime < 2000) {
    drawLine(p1, p2);
  }

#else

  float uStart = 0.48;
  float uEnd = 0.4;
  float delta = 0.002;
  
  for (float u = uStart; u > uEnd; u -= delta) {
    p1.x = u;
    p2.x = uStart + (0.5 + (0.5-uStart) - u);
    drawLine(p1, p2);
  } 

  for (float u = uEnd; u < uStart; u += delta) {
    p1.x = u;
    p2.x = uStart + (0.5 + (0.5-uStart) - u);
    drawLine(p1, p2);
  } 

#endif

  delay(1000);

}


void drawLine(Point p1, Point p2) {
  goToPoint(p1);
  goToPoint(p2);
}

void goToPoint(Point p) {  
  int angle = angleForPosition(p.x);
//  Serial.print(p.x);
//  Serial.print(": ");
//  Serial.println(angle);
  servo.write(angle);
  delay(MIN_SERVO_DELAY);
}

uint8_t angleForPosition(float position) {
    return position * 90.0; 
}



