#include <Adafruit_NeoPixel.h>
#include "LEDStripParticleEmitter.h"

#define NUM_PIXELS 30
#define NUM_PARTICLES 30

#define FPS 210
#define PIN 11
#define MAX_COLOR 255   // max 255
#define MIN_COLOR 3
#define MAX_VELOCITY 0.015
#define MAX_BATCH_MILLIS 3000
#define MIN_BATCH_MILLIS 1000
#define EMITTER_TRANSIT_MILLIS 3000 // 235000  // millis to reach other side
#define MILLIS_PER_FRAME (1000 / FPS)
#define MAX_THROBBER  0.1618 //0.3236
#define MIN_THROBBER -0.6472
#define THROBBER_DELTA -0.000035  //-0.000025

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
ParticleEmitter emitter = ParticleEmitter(NUM_PIXELS, MAX_COLOR);

float throbber = MAX_THROBBER;
float throbberDelta = THROBBER_DELTA;
float emitterTransitStartMillis = 0; 
float emitterTransitDirection = 1;

unsigned long frameStartMillis = 0;
uint8_t randomRedColor = 0;
uint8_t randomGreenColor = 0;
uint8_t randomBlueColor = 0;

void setCoordColor(Coord3D coord, uint32_t color);

////////////////////////////////////
////////////////////////////////////


  // Pins for the sensor and LCD.
uint8_t pirPin1 = 5;
uint8_t pirPin2 = 6;
uint8_t pirPin3 = 7;

#define CUM_MOTION_CHECK_MILLIS 300.0
#define CUM_MOTION_MAX_SEC 60  // seconds
const float cumMotionDelta = (CUM_MOTION_CHECK_MILLIS / 600.0); 

double cumMotion1 = 0; 
double cumMotion2 = 0; 
double cumMotion3 = 0; 
long lastCumMotionCheckMillis = millis();

uint8_t lightPin1 = 8;
uint8_t lightPin2 = 9;
uint8_t lightPin3 = 10;

//uint8_t mode = 0;  // all off
//uint8_t mode = 1;  // all on
uint8_t mode = 2;    // motion sensors

const int lightSensorPin = 0;
const int lightSensedPin = 13;
int high = 0, low = 1023, lightLevel = high;
boolean daytime = 0;

uint8_t motionDetectionsPerMinute = 0;
uint8_t motionDetections = 0;
long lastMotionDetectedAt = millis();




// Debounce the pIR sensor. 
// The transition from HIGH to LOW is noisy.
long lastInterrupt = millis();
long debounce = 5;

void setup() {
  Serial.begin(9600);

  pinMode(lightSensedPin, OUTPUT);
  pinMode(lightPin1, OUTPUT);
  pinMode(lightPin2, OUTPUT);
  pinMode(lightPin3, OUTPUT);
  
  pinMode(pirPin1, INPUT);
  digitalWrite(pirPin1, LOW);
  pinMode(pirPin2, INPUT);
  digitalWrite(pirPin2, LOW);
  pinMode(pirPin3, INPUT);
  digitalWrite(pirPin3, LOW);
  
  // Attaching interrupt 1 (pin 3 on Uno/Mega) to spotted() method.
//  attachInterrupt(0, spotted, CHANGE);

  // PARTICLES SETUP

    strip.begin();
    strip.show();
    
    emitter.respawnOnOtherSide = true;
    emitter.threed = true;
    emitter.numParticles = NUM_PARTICLES;
    emitter.maxVelocity = MAX_VELOCITY;
    
  //  Serial.println(emitter.maxColor);
    emitterTransitStartMillis = millis();

}

uint8_t light1State = LOW;
uint8_t light2State = LOW;
uint8_t light3State = LOW;
uint8_t motion1State = LOW;
uint8_t motion2State = LOW;
uint8_t motion3State = LOW;

boolean senseMotionAndLight() {
  if ((millis() - lastCumMotionCheckMillis) > CUM_MOTION_CHECK_MILLIS) {
    motion1State = digitalRead(pirPin1);
    motion2State = digitalRead(pirPin2);
    motion3State = digitalRead(pirPin3);    

//    Serial.print(daytime);
//    Serial.print(" day :: ");
//    Serial.print(motion1State);
//    Serial.print(" ");
//    Serial.print(motion2State);
//    Serial.print(" ");
//    Serial.print(motion3State);
//
//    Serial.print(" --- ");
//    Serial.print(cumMotion1);
//    Serial.print(" ");
//    Serial.print(cumMotion2);
//    Serial.print(" ");
//    Serial.print(cumMotion3);
//
//    Serial.print(" --- ");
//    Serial.println(cumMotionDelta);
    
    lastCumMotionCheckMillis = millis();
        
    if (motion1State == HIGH && cumMotion1 < CUM_MOTION_MAX_SEC) {
      cumMotion1 += cumMotionDelta;
    }
    else if (motion1State == LOW && cumMotion1 > 0) {
      cumMotion1 -= cumMotionDelta/3;
    }
    
    if (motion2State == HIGH && cumMotion2 < CUM_MOTION_MAX_SEC) {
      cumMotion2 += cumMotionDelta;
    }
    else if (motion2State == LOW && cumMotion2 > 0) {
      cumMotion2 -= cumMotionDelta/3;
    }

    if (motion3State == HIGH && cumMotion3 < CUM_MOTION_MAX_SEC) {
      cumMotion3 += cumMotionDelta;
    }
    else if (motion3State == LOW && cumMotion3 > 0) {
      cumMotion3 -= cumMotionDelta/3;
    }

//    Serial.print(daytime);
//    Serial.print(" day :: ");
//    Serial.print(cumMotion1);
//    Serial.print(" ");
//    Serial.print(cumMotion2);
//    Serial.print(" ");
//    Serial.println(cumMotion3);
    
    digitalWrite(lightPin1, !light1State);
    digitalWrite(lightPin2, !light2State);
    digitalWrite(lightPin3, !light3State);  
    
    
    // particles
    float cumMotion = cumMotion1 + cumMotion2 + cumMotion3;    
    emitter.numParticles = (NUM_PARTICLES*0.5) + ((NUM_PARTICLES*0.5) * (cumMotion / CUM_MOTION_MAX_SEC));
//    emitter.maxVelocity = (MAX_VELOCITY*0.25) * ((MAX_VELOCITY*0.75) * (cumMotion / CUM_MOTION_MAX_SEC));
//    throbberDelta = (THROBBER_DELTA*0.25) * ((THROBBER_DELTA*0.75) * (cumMotion / CUM_MOTION_MAX_SEC));

//    Serial.print("** ");
//    Serial.print(cumMotion / CUM_MOTION_MAX_SEC);
//    Serial.print(" : ");
//    Serial.println(emitter.numParticles);

  }

  lightLevel = analogRead(lightSensorPin);
//  manualTune();  
  autoTune();

  if (lightLevel > 200) {
    daytime = 1;
    mode = 0;
    cumMotion1 = cumMotion2 = cumMotion3 = 0;
//    Serial.println(lightLevel);
  }
  else {
    daytime = 0;
    mode = 2;
  }

  
  if (mode == 0) {
    // always off  
    light1State = light2State = light3State = LOW;
  }
  else if (mode == 1) {
    // always on
    light1State = light2State = light3State = HIGH;
  }
  else if (mode == 2) {
    // Simple direct motion control of lights. Use hardware adjusters.
    light1State = motion1State;
    light2State = motion2State;
    light3State = motion3State;
  }
  else {
    // cumulative motion (doesn't work very well)
    light1State = (cumMotion1 > 0 ? HIGH : LOW);
    light2State = (cumMotion2 > 0 ? HIGH : LOW);
    light3State = (cumMotion3 > 0 ? HIGH : LOW);
  }

  return daytime;
}

// Called by interrupt when signal on pin 3 goes from HIGH to LOW or LOW to HIGH.
void spotted() {
  // The pIR sensor doesn't cleanly go from HIGH to LOW. This method will be called
  // twice on LOW. If the method has been called twice within the debounce period
  // just return.
//  if((millis() - lastInterrupt) < debounce) return;
  
  // LOW to HIGH transition (motion sensed)
  if(digitalRead(pirPin1) == HIGH) {
//    Serial.println("ON");
  }
  else
  {
    // HIGH to LOW trasition (no motion sensed)
    Serial.println("-");
  }
  
  // Save the time we were last called so we can debounce the signal.
  lastInterrupt = millis();
}


void manualTune()
{
  // As we mentioned above, the light-sensing circuit we built
  // won't have a range all the way from 0 to 1023. It will likely
  // be more like 300 (dark) to 800 (light). If you run this sketch
  // as-is, the LED won't fully turn off, even in the dark.
  
  // You can accommodate the reduced range by manually 
  // tweaking the "from" range numbers in the map() function.
  // Here we're using the full range of 0 to 1023.
  // Try manually changing this to a smaller range (300 to 800
  // is a good guess), and try it out again. If the LED doesn't
  // go completely out, make the low number larger. If the LED
  // is always too bright, make the high number smaller.

  // Remember you're JUST changing the 0, 1023 in the line below!

  lightLevel = map(lightLevel, 0, 1023, 0, 255);
  lightLevel = constrain(lightLevel, 0, 255);

  // Now we'll return to the main loop(), and send lightLevel
  // to the LED.
} 


void autoTune()
{
  // As we mentioned above, the light-sensing circuit we built
  // won't have a range all the way from 0 to 1023. It will likely
  // be more like 300 (dark) to 800 (light).
  
  // In the manualTune() function above, you need to repeatedly
  // change the values and try the program again until it works.
  // But why should you have to do that work? You've got a
  // computer in your hands that can figure things out for itself!

  // In this function, the Arduino will keep track of the highest
  // and lowest values that we're reading from analogRead().

  // If you look at the top of the sketch, you'll see that we've
  // initialized "low" to be 1023. We'll save anything we read
  // that's lower than that:
  
  if (lightLevel < low)
  {
    low = lightLevel;
  }

  // We also initialized "high" to be 0. We'll save anything
  // we read that's higher than that:
  
  if (lightLevel > high)
  {
    high = lightLevel;
  }
  
  // Once we have the highest and lowest values, we can stick them
  // directly into the map() function. No manual tweaking needed!
  
  // One trick we'll do is to add a small offset to low and high,
  // to ensure that the LED is fully-off and fully-on at the limits
  // (otherwise it might flicker a little bit).
  
  lightLevel = map(lightLevel, low+30, high-30, 0, 255);
  lightLevel = 255 - constrain(lightLevel, 0, 255);
  
  // Now we'll return to the main loop(), and send lightLevel
  // to the LED.
}

void goDark() {
  // Take the relay HIGH which turns the light off.
  light1State = light2State = light3State = HIGH;
  
  digitalWrite(lightPin1, light1State);
  digitalWrite(lightPin2, light2State);
  digitalWrite(lightPin3, light3State);  

  for (uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, 0, 0, 0);
  }
  strip.show();
}



////////////////////////////////////
// particles follow...
////////////////////////////////////


void loop() {  
  boolean drawTails = false; //(random(10) > 3);
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;
  unsigned long batchMillis = (MAX_BATCH_MILLIS - MIN_BATCH_MILLIS) * (random(100) / 100) + MIN_BATCH_MILLIS;  
  emitter.respawnOnOtherSide = !emitter.respawnOnOtherSide;
  emitter.numParticles = random(NUM_PARTICLES) + 1;

  if (senseMotionAndLight()) {
    goDark();
    return;
  }

  while (elapsedMillis <= batchMillis) {
    frameStartMillis = millis();

    if (senseMotionAndLight()) {
      goDark();
      return;
    }
    
//    float emitterTransitOffset = ((EMITTER_TRANSIT_MILLIS - (millis() - emitterTransitStartMillis)) / EMITTER_TRANSIT_MILLIS);  
//    emitter.stripPosition = (emitterTransitDirection > 0 ? emitterTransitOffset : 1 - emitterTransitOffset);
//    if (emitter.stripPosition >= 1.0 || emitter.stripPosition <= 0.0) {
//      emitterTransitStartMillis = millis();
//      emitterTransitDirection *= -1;
//    }

    // Pulse the whole strip
    for (uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, 
                          strip.Color(randomRedColor*(throbber<0?0:throbber),
                                      randomGreenColor*(throbber<0?0:throbber),
                                      randomBlueColor*(throbber<0?0:throbber)));
      
      throbber += throbberDelta;
      //Serial.println(throbber);
      if (throbber >= MAX_THROBBER || throbber <= MIN_THROBBER) {
        
        throbberDelta *= -1;
        
        if (throbber <= MIN_THROBBER) { 
          // Change the strip color
          randomRedColor = random(MAX_COLOR);
          randomGreenColor = random(MAX_COLOR);
          randomBlueColor = random(MAX_COLOR);

          // Slightly vary the throb time
          throbberDelta += (random(2) == 0 ? 1 : -1) * (random(100) / 100 * 0.005);    

          // Change the particle velocities
          // There's a small chance of high speed
//          emitter.maxVelocity = (random(100) / 100.0 * 0.006) * (random(6) == 0 ? 2 : 1) + 0.004;
          emitter.maxVelocity = (random(30) / 100.0 * MAX_VELOCITY + MAX_VELOCITY*0.7);
          float scale = random(6);
          if (scale == 0) {
            scale = 0.33;
          }
          else if (scale == 1) {
            scale = 3.0;
          }
          else {
            scale = 1.0;
          }
          
          emitter.maxVelocity *= scale;
       }
     }
    }

    // Draw each particle
    for (int i=0; i < emitter.numParticles; i++) {

      // Update this particle's position
      boolean respawn = (elapsedMillis > (batchMillis * 0.5));
      Particle prt = emitter.updateParticle(i, respawn);

      uint8_t tailLength = (drawTails ? abs(prt.velocity.x * 8) : 1);
      int16_t startSlot = NUM_PIXELS * prt.coord.x;
      int16_t currentSlot = startSlot;
      int16_t oldSlot = currentSlot;
      
      // Draw the particle and its tail
      // High velocity particles have longer tails
      for (int z=0; z < tailLength; z++) { 
        
        // Taper the tail fade  
        float colorScale = ((tailLength - (z * 0.25)) / tailLength);

        if (z == 0 && prt.dimmed) {
          // Flicker the first particle
          colorScale *= (random(50) / 100) + 0.05;
        }      

        if (colorScale < 0.05) {
          colorScale = 0.05;
        }

        // Draw particle
//        strip.setPixelColor(currentSlot, 
//                            strip.Color(prt.redColor*colorScale, 
//                                        prt.blueColor*colorScale, 
//                                        prt.greenColor*colorScale));

        if (emitter.threed) {
          byte colorMode = 3;
          
          switch (colorMode) {
            case 1:
              colorScale = 1.0;
              setCoordColor(prt.coord, strip.Color(prt.redColor*colorScale*(1.0 - prt.coord.z),
                                                  (prt.coord.z < 0.33 ? MAX_COLOR*prt.coord.z : 0), 
                                                  (prt.coord.z > 0.66 ? MAX_COLOR*prt.coord.z : 0)));
              break;
            case 2:
              colorScale = 1.0;
              setCoordColor(prt.coord, strip.Color(MAX_COLOR*colorScale*(1.0 - prt.coord.z),
                                                  prt.greenColor*colorScale, 
                                                  MAX_COLOR*colorScale*prt.coord.z));
              break;
            case 3:
              colorScale = (1.0 - prt.coord.z);
              setCoordColor(prt.coord, strip.Color(prt.redColor*colorScale, 
                                                   prt.greenColor*colorScale, 
                                                   prt.blueColor*colorScale));
              break;
          }
        }
        else {
          setCoordColor(prt.coord, 
                        strip.Color(prt.redColor*colorScale, 
                                    prt.greenColor*colorScale, 
                                    prt.blueColor*colorScale));
        }

        oldSlot = currentSlot;
        currentSlot = startSlot + ((z+1) * (prt.velocity.x > 0 ? -1 : 1));
      }

      // Terminate the tail
//      strip.setPixelColor(oldSlot, strip.Color(1,0,1));
    }

    // Draw the spawn point
//    uint8_t spawnColor = (MAX_COLOR / 200 * random(100) / 100);
//    strip.setPixelColor(emitter.stripPosition*NUM_PIXELS, 
//                        strip.Color(random(2) == 0 ? 0 : random(MAX_COLOR),
//                                    random(2) == 0 ? 0 : random(MAX_COLOR), 
//                                    random(2) == 0 ? 0 : random(MAX_COLOR)));

    uint16_t frameElapsedMillis = millis() - frameStartMillis;
    uint16_t frameDelayMillis = 0;
    
    if (MILLIS_PER_FRAME > frameElapsedMillis) {
      frameDelayMillis = MILLIS_PER_FRAME - frameElapsedMillis;
    }    

    delay(frameDelayMillis);

    strip.show();
    elapsedMillis = millis() - startMillis;
  }
}

uint8_t numColumns = 16;
uint8_t numRows = 8;

void setCoordColor(Coord3D coord, uint32_t color) {
  // Given x,y,z convert to pixel number.
  
  uint8_t column = coord.x * numColumns;
  uint8_t row = coord.y * numRows;

  // 0,0 is in bottom left corner
  uint8_t index;
  
  
  
  if (row % 2 == 0) {
    index = column + (row * numColumns);
  }
  else {
    int offset = 0;
    offset = numColumns - ((column * 2) + 1) ;
    index = ((row * numColumns) + column) + offset;
  } 
 
//      Serial.print(coord.x);
//      Serial.print(",");
//      Serial.print(coord.y);
//      Serial.print(":");
//      Serial.println(index);
      
  strip.setPixelColor(index, color); 
 
  
}


