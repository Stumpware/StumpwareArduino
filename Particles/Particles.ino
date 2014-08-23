#include <Adafruit_NeoPixel.h>
#include "LEDStripParticleEmitter.h"

#define NUM_PIXELS 90
#define NUM_PARTICLES 12

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
#define THROBBER_DELTA -0.000025

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

void setup() {
//  Serial.begin(9600);
  strip.begin();
  strip.show();
  
  emitter.respawnOnOtherSide = true;
  emitter.threed = true;
  emitter.numParticles = NUM_PARTICLES;
  emitter.maxVelocity = MAX_VELOCITY;
  
//  Serial.println(emitter.maxColor);
  emitterTransitStartMillis = millis();
}

void loop() {  
  boolean drawTails = false; //(random(10) > 3);
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;
  unsigned long batchMillis = (MAX_BATCH_MILLIS - MIN_BATCH_MILLIS) * (random(100) / 100) + MIN_BATCH_MILLIS;  
  emitter.respawnOnOtherSide = !emitter.respawnOnOtherSide;
  emitter.numParticles = random(NUM_PARTICLES) + 1;

  while (elapsedMillis <= batchMillis) {
    frameStartMillis = millis();
    
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


