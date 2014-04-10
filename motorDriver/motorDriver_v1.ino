// Reed switch repetition counter
 
int sense0 = 2;
int counter0 = 0;
long lastDebounce0 = 0;
long debounceDelay = 10;    // Ignore bounces under 1/2 second
unsigned short interruptsPerRotationLeft = 26;
unsigned short millisInMinute = 60000;
unsigned short speedTimeout = 1000;

double motor0Rpm = 0;
long motor0LastReadingTime = 0;
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
  Serial.print(" Done Min speed, RPM: ");
  Serial.println(motor0Rpm);
  
  int maxSpeed = findMaxSpeed();
  Serial.print(maxSpeed);
  Serial.print(" Done max speed, RPM: ");
  Serial.println(motor0Rpm);
  //TODO: calculate average speed for a few millis for both max and min
  
}
 
void loop() {
  // Nothing here
  Serial.print(motor0Rpm);
  Serial.print(", ");
  Serial.println(motor0IsSpinning);
  if(motor0IsSpinning) {
    isMotorActive(&motor0LastReadingTime, &motor0Rpm, &motor0IsSpinning);
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
    delay(10);
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

void setMotor(int speed, boolean reverse)
{
  analogWrite(speedPin, speed);
  digitalWrite(in1Pin, ! reverse);
  digitalWrite(in2Pin, reverse);
}
 
void isMotorActive(long *lastSpinTime, double *motorRpm, boolean *motorActiveFlag) {
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
    long currentTime = millis();
    long deltaTime = currentTime - lastDebounce0;
 
    //Calculate speed
    long fullRotationTime = deltaTime * interruptsPerRotationLeft;
    double rpm = millisInMinute / fullRotationTime;
    motor0Rpm = rpm;
    motor0IsSpinning = true;
 
    lastDebounce0 = millis(); //Recalc current time to account for processing times
    motor0LastReadingTime = lastDebounce0;
  //}
}
