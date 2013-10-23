#include <Adafruit_NeoPixel.h>
#include "ParticleEmitter.h"

#define PIN 6
#define MIN_COLOR 100
#define MAX_COLOR 255
#define NUM_PIXELS 300
#define MAX_EMITTER_POSITION_MILLIS 3000
#define MIN_EMITTER_POSITION_MILLIS 1000
#define FPS 40
#define MILLIS_PER_FRAME (1000 / FPS)

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, PIN, NEO_GRB + NEO_KHZ800);
ParticleEmitter emitter = ParticleEmitter(NUM_PIXELS);

#define MAX_THROBBER 1.0
#define MIN_THROBBER -0.75
float throbber = MAX_THROBBER;
float throbberDelta = -0.00006;

uint8_t randomRedColor = 0;
uint8_t randomGreenColor = 0;
uint8_t randomBlueColor = 0;


void setup() {
//  Serial.begin(9600);
  randomSeed(analogRead(0));
  strip.begin();
  strip.show();
}

unsigned long frameStartMillis = 0;

void loop() {  
  boolean drawTails = (random(2) == 0);
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;

  unsigned long positionMillis = (MAX_EMITTER_POSITION_MILLIS - MIN_EMITTER_POSITION_MILLIS) * (random(100) / 100) + MIN_EMITTER_POSITION_MILLIS;
  emitter.stripPosition = random(100) / 100.0;

  while (elapsedMillis <= positionMillis) {
    frameStartMillis = millis();

    // Pulse the whole strip
    for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, 
                          strip.Color(randomRedColor*(throbber<0?0:throbber),
                                      randomGreenColor*(throbber<0?0:throbber),
                                      randomBlueColor*(throbber<0?0:throbber)));
      
      throbber += throbberDelta;

      if (throbber >= MAX_THROBBER || throbber <= MIN_THROBBER) {
        throbberDelta *= -1;
        
        if (throbber <= MIN_THROBBER) { 
          // Change the strip color
          randomRedColor = random(MIN_COLOR);
          randomGreenColor = random(MIN_COLOR);
          randomBlueColor = random(MIN_COLOR);

          // Slightly vary the throb time
          throbberDelta += (random(2) == 0 ? 1 : -1) * (random(100) / 100 * 0.001);    

          // Change the particle velocities
          // There's a small chance of high speed
          emitter.maxVelocity = (random(100) / 100.0 * 0.006) * (random(6) == 0 ? 2 : 1) + 0.004;
        }
      }            
    }

    // Draw each particle
    for (int i=0; i < emitter.numParticles; i++) {

      // Update this particle's position
      boolean respawn = (elapsedMillis > (positionMillis * 0.33));
      particle prt = emitter.updateParticle(i, respawn);

      uint8_t tailLength = (drawTails ? abs(prt.velocity * 15) : 2);
      uint16_t startSlot = NUM_PIXELS * prt.currentStripPosition;
      uint16_t currentSlot = startSlot;
      uint16_t oldSlot = currentSlot;
      
      // Draw the particle and its tail
      // High velocity particles have longer tails
      for (int z=0; z < tailLength; z++) { 
        
        // Taper the tail fade  
        float colorScale = ((tailLength-z*0.997) / tailLength);

        if (z == 0 && prt.dimmed) {
          // Flicker the first particle
          colorScale *= (0.5 * random(100) / 100) + 0.05;
        }      

        if (colorScale < 0.15) { colorScale = 0.15; }

        strip.setPixelColor(currentSlot, 
                            strip.Color(prt.redColor*colorScale, 
                                        prt.blueColor*colorScale, 
                                        prt.greenColor*colorScale));

        oldSlot = currentSlot;
        currentSlot = startSlot + ((z+1) * (prt.velocity > 0 ? -1 : 1));
      }

      // Terminate the tail
      strip.setPixelColor(oldSlot, strip.Color(3,3,3));
    }

    // Draw the spawn point
    uint8_t spawnColor = MIN_COLOR * random(100) / 100;
    strip.setPixelColor(emitter.stripPosition*NUM_PIXELS, 
                        strip.Color(random(2) == 0 ? 0 : spawnColor,
                                    random(2) == 0 ? 0 : spawnColor, 
                                    random(2) == 0 ? 0 : spawnColor));
    
    uint16_t frameElapsedMillis = millis() - frameStartMillis;
    uint16_t frameDelayMillis = 0;
    
    if (MILLIS_PER_FRAME > frameElapsedMillis)
    {
      frameDelayMillis = MILLIS_PER_FRAME - frameElapsedMillis;
    }    

    delay(frameDelayMillis);

    strip.show();
    elapsedMillis = millis() - startMillis;
  }
}

