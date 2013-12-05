#include <OneWire.h>
#include <MemoryFree.h>
#include <XBee.h>
 
XBee xbee = XBee(); 
TxStatusResponse txStatus = TxStatusResponse();
 
int led = 13;
int DS18S20_Pin = 11; //DS18S20 Signal pin on digital 2

//Temperature chip i/o
OneWire ds(DS18S20_Pin); // on digital pin 2

int aPinIn = 0;            // Analogue Input on Arduino
int val = 0;               // The raw analogue value
float Vout = 0.0;          // Voltage at point between resistors
                           // (relative to ground)
float Vin = 5.0;           // Vcc (5 Volts)
float Rknown = 1000.0;    // The known resistor (10 kohms)
float Runknown = 0.0;

char tempKey = 'T';
char depthKey = 'D';
char delimiter = ',';

void setup(){
  pinMode(led, OUTPUT);
  xbee.begin(9600);
  Serial.begin(9600);
}
 
void loop(){
  Serial.println("Looping");
  int i = 0;
  float depthSum = 0;
  float tempSum = 0;
  
  for(int i = 0; i <= 14;) {
    val = analogRead(aPinIn);              // Read in val (0-1023)
    Vout = (Vin/1024.0) * float(val);      // Convert to voltage
    Runknown = Rknown*((Vin/Vout) - 1);    // Calculate Runknown
         
    // TEMP
    float temperature = getTemp();
    float tempF = (temperature * 1.8) + 32;
    
    //if(tempF > 0 && tempF != 185) {
      tempSum += tempF;
      depthSum += Runknown;
      i++;
    //}
    flashLed(led,2,100);
    delay(500);                           // delay for readability 
  }
  
  depthSum /= 15;
  tempSum /= 15;
  
  // Join string in a single declaration if memory is an issue
  String payload = tempKey + String(tempSum) + delimiter + 
            depthKey + String(depthSum) + delimiter;
 
  //send message to xbee
  boolean retrySend = true;
  int retryCount = 0;
  while(retrySend && retryCount <= 10) {
    retrySend = !sendMessage(payload);
    retryCount++;
    delay(1000);
  }
  
  Serial.println("After to array");
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());
  
  delay(10000);
}

boolean sendMessage(String payload){
  
  int payloadLenght = payload.length() + 1;
  byte payloadArray[payloadLenght];
  payload.getBytes(payloadArray, payloadLenght);
  
  Serial.println("In method .. ");
  Serial.println(sizeof(payloadArray));
    
  Tx16Request tx = Tx16Request(0x01, payloadArray, sizeof(payloadArray));
  xbee.send(tx);
  
  if (xbee.readPacket(1000)) {
        // got a response!
        Serial.println("Got response");
        // should be a znet tx status                   
        if (xbee.getResponse().getApiId() == TX_STATUS_RESPONSE) {
           xbee.getResponse().getZBTxStatusResponse(txStatus);
 
           // get the delivery status, the fifth byte
           if (txStatus.getStatus() == SUCCESS) {
                // success.  time to celebrate
                Serial.println("Got response");
                return true;
           } else {
                // the remote XBee did not receive our packet. is it powered on?
                 Serial.println("Wrong response :(");
                 return false;
           }
        }      
    } else {
      Serial.println("No response");
      // local XBee did not provide a timely TX Status Response -- should not happen
    }
    
    return false;

}

float getTemp(){
 //returns the temperature from one DS18S20 in DEG Celsius
 
 byte data[12];
 byte addr[8];
 
 if ( !ds.search(addr)) {
   //no more sensors on chain, reset search
   ds.reset_search();
   return -1000;
 }
 
 if ( OneWire::crc8( addr, 7) != addr[7]) {
   Serial.println("CRC is not valid!");
   return -1000;
 }
 
 if ( addr[0] != 0x10 && addr[0] != 0x28) {
   Serial.print("Device is not recognized");
   return -1000;
 }
 
 ds.reset();
 ds.select(addr);
 ds.write(0x44,1); // start conversion, with parasite power on at the end
 
 byte present = ds.reset();
 ds.select(addr);
 ds.write(0xBE); // Read Scratchpad
 
 
 for (int i = 0; i < 9; i++) { // we need 9 bytes
  data[i] = ds.read();
 }
 
 ds.reset_search();
 
 byte MSB = data[1];
 byte LSB = data[0];
 
 float tempRead = ((MSB << 8) | LSB); //using two's compliment
 float TemperatureSum = tempRead / 16;
 
 return TemperatureSum;
 
}

void flashLed(int pin, int times, int wait) {
 
    for (int i = 0; i < times; i++) {
      digitalWrite(pin, HIGH);
      delay(wait);
      digitalWrite(pin, LOW);
 
      if (i + 1 < times) {
        delay(wait);
      }
    }
}
