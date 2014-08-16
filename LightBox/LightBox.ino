


// Pins for the sensor and LCD.
uint8_t pirPin1 = 5;
uint8_t pirPin2 = 6;
uint8_t pirPin3 = 7;

uint8_t lightPin1 = 8;
uint8_t lightPin2 = 9;
uint8_t lightPin3 = 10;

//uint8_t mode = 0;  // all off
//uint8_t mode = 1;  // all on
uint8_t mode = 2;    // motion sensors


const int lightSensorPin = 0;
const int lightSensedPin = 13;
int lightLevel, high = 0, low = 1023;




// Debounce the pIR sensor. 
// The transition from HIGH to LOW is noisy.
long lastInterrupt = millis();
long debounce = 5;

void setup() {
  Serial.begin(9600);
  Serial.println("setup");

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
}

uint8_t light1State = LOW;
uint8_t light2State = LOW;
uint8_t light3State = LOW;

void loop() {

  lightLevel = analogRead(lightSensorPin);
//  manualTune();  
  autoTune();
//  analogWrite(lightSensedPin, lightLevel);
  
  if (lightLevel < (low / 2)) {
//    mode = 0;
//    Serial.println(lightLevel);
    digitalWrite(lightSensedPin, LOW);
  }
  else {
//    mode = 2;
    digitalWrite(lightSensedPin, HIGH);
 
//    Serial.println(lightLevel);
    
  }

  
  if (mode == 0) {
    // always off  
    light1State = light2State = light3State = LOW;
  }
  else if (mode == 1) {
    // always on
    light1State = light2State = light3State = HIGH;
  }
  else {
    // Simple direct motion control of lights.
    light1State = !digitalRead(pirPin1);
    light2State = !digitalRead(pirPin2);
    light3State = !digitalRead(pirPin3);
  }

  digitalWrite(lightPin1, light1State);
  digitalWrite(lightPin2, light2State);
  digitalWrite(lightPin3, light3State);
  
}

// Called by interrupt when signal on pin 3 goes from HIGH to LOW or LOW to HIGH.
void spotted() {
  // The pIR sensor doesn't cleanly go from HIGH to LOW. This method will be called
  // twice on LOW. If the method has been called twice within the debounce period
  // just return.
//  if((millis() - lastInterrupt) < debounce) return;
  
  // LOW to HIGH transition (motion sensed)
  if(digitalRead(pirPin1) == HIGH) {
    Serial.println("ON");
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
  lightLevel = constrain(lightLevel, 0, 255);
  
  // Now we'll return to the main loop(), and send lightLevel
  // to the LED.
}

