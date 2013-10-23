

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  pinMode(2, INPUT);
  pinMode(13, OUTPUT);
  pinMode(9, OUTPUT);
  
}

void loop() {
  // put your main code here, to run repeatedly: 

    if (digitalRead(2)) {
      digitalWrite(13, HIGH);
      tone(9, 300, 100);
    }
    else
    {
      noTone(9);
      digitalWrite(13, LOW`);
    }
    
    
}
