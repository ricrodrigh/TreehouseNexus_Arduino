// Reed switch repetition counter
 
int sense0 = 2;
unsigned long lastDebounce0 = 0;
short debounceDelay = 10;    // Ignore bounces under 1/2 second
unsigned short interruptsPerRotationLeft = 26;
unsigned short speedTimeout = 1000;

unsigned long motor0RotationTime = 0;
unsigned long motor0LastReadingTime = 0;
boolean motor0IsSpinning = false;

int speedPin = 11;
int in1Pin = 10;
int in2Pin = 9;
int speed = 500;
boolean reverse = true;
 
void setup() {
  Serial.begin(9600);
  pinMode(sense0, INPUT);
  digitalWrite(sense0, HIGH);
  attachInterrupt(0, trigger0, FALLING);
  
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(speedPin, OUTPUT);
  
  int minSpeed = findMinSpeed();
  
  Serial.print(minSpeed);
  Serial.print(" Done, Min rotation time in millis: ");
  Serial.println(motor0RotationTime);
  
  int maxSpeed = findMaxSpeed();
  Serial.print(maxSpeed);
  Serial.print(" Done, max rotation time in millis ");
  Serial.println(motor0RotationTime);
  //TODO: calculate average speed for a few millis for both max and min
  
}
 
void loop() {
  // Nothing here
  Serial.print(motor0RotationTime);
  Serial.print(", ");
  Serial.println(motor0IsSpinning);
  if(motor0IsSpinning) {
    isMotorActive(&motor0LastReadingTime, &motor0RotationTime, &motor0IsSpinning);
  }
  
  delay(800);
}

int findMinSpeed() {
  int minSpeed = 0;
  do {
    Serial.print("Finding Min speed");
    Serial.println(minSpeed);
    setMotor(minSpeed, true);
    minSpeed++;
    delay(50);
  } while (!motor0IsSpinning && minSpeed < 255);
  
  delay(1000);
  return minSpeed;
}

int findMaxSpeed() {
  //Verify that 255 makes it spin
  int maxSpeed = 255;
  setMotor(maxSpeed, true);
  if(motor0IsSpinning) {
    delay(1000);
    return maxSpeed;
  }
  
  return 255;
}

unsigned long getAverageRotationTime(int numberOfSamples, unsigned long *motorRotationTime) {
  unsigned long averageRotationTime = 0;
  if(numberOfSamples == 0) {
    return averageRotationTime;
  }
  for(int i = 0; i < numberOfSamples; i++) {
      averageRotationTime = averageRotationTime + *motorRotationTime;
  }
  
  return averageRotationTime / numberOfSamples;
}

void setMotor(int speed, boolean reverse)
{
  analogWrite(speedPin, speed);
  digitalWrite(in1Pin, ! reverse);
  digitalWrite(in2Pin, reverse);
}
 
void isMotorActive(unsigned long *lastSpinTime, unsigned long *motorRpm, boolean *motorActiveFlag) {
  float currentTime = millis();
  float lastTimePlusTimeout = *lastSpinTime + speedTimeout;
  if(currentTime > lastTimePlusTimeout) {
    Serial.println("Wheel inactive");
    *motorActiveFlag = false;
    *motorRpm = 0;
  }
}
 
void trigger0() {
  //if( (millis() - lastDebounce0) > debounceDelay){
    long deltaTime = millis() - lastDebounce0;
 
    //Calculate speed
    motor0RotationTime = deltaTime * interruptsPerRotationLeft;
    motor0IsSpinning = true;
 
    lastDebounce0 = millis(); //Recalc current time to account for processing times
    motor0LastReadingTime = lastDebounce0;
  //}
}
