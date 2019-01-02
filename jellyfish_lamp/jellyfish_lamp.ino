#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <TimerOne.h> 

#define CE_PIN  7
#define CSN_PIN 8

Adafruit_NeoPixel ring;
const int ringSize = 16;
const int ringPin = 2;

const byte thisSlaveAddress[5] = {'R','x','A','A','A'};

long personalityTimerInterval = 0;
int personalityProgramType = 0;
int personalitySpeed = 0;
bool personalityState =false;
int currentState = 0;
RF24 radio(CE_PIN, CSN_PIN);

char dataReceived[6]; // this must match dataToSend in the TX
bool newData = false;
bool lampIsOn = false;
volatile bool stateChanged = false;
//long timerPeriod = 1000000;

void timerIsr(){
  switch(personalityProgramType){
    case 0:
      colorCycle();
      break;
    case 1:
      colorBlink();
      break;
    case 2:
      if(!personalityState){
        colorMix();
      }
      break;
  }
}

void colorCycle(){
  setAllPixels(random(0,255),random(0,255),random(0,255));
  ring.show();
}

void colorBlink(){
  if(personalityState){
    setAllPixels(random(0,0),random(0,0),random(0,0));
    personalityState = false;
  }else{
    setAllPixels(random(0,255),random(0,255),random(0,255));
    personalityState = true;
  }  
  ring.show();
}

void colorMix(){
  personalityState = true;
  int r = random(0,255);
  int g = random(0,255);
  int b = random(0,255);
  
  for(int i=0; i<ringSize; i=i+2){
    ring.setPixelColor(i,r,g,b);
    ring.show();
    delay(personalitySpeed);
  }
  personalityState = false;
}

void setPersonality(){

  personalityTimerInterval = random(5,50) * 100000; //.5 to 5 seconds
  personalityProgramType = random(0,3);
  personalitySpeed = random(100,1000);

/*
  personalityTimerInterval = 1000000; //.5 to 5 seconds
  personalityProgramType = 0;
  personalitySpeed = 100;  
*/
/*
  Serial.print("Timer Interval - ");
  Serial.println(personalityTimerInterval);

  Serial.print("Program Type - ");
  Serial.println(personalityProgramType);

  Serial.print("Speed - ");
  Serial.println(personalitySpeed);  
 */   
}
void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(0));
  


  pinMode(ringPin, OUTPUT);
  ring = Adafruit_NeoPixel(ringSize, ringPin, NEO_GRB + NEO_KHZ800);
  ring.begin();
  setAllPixels(0,0,0);
  ring.show();


  Timer1.initialize(personalityTimerInterval);
  Timer1.pwm(9, 512); 
  Timer1.attachInterrupt( timerIsr );
  Timer1.stop();
  
  Serial.println("Starting");
  radio.begin();
  //radio.setDataRate( RF24_250KBPS );
  radio.setChannel(111);
  radio.setAutoAck(false);
  radio.openReadingPipe(1, thisSlaveAddress);
  radio.startListening();
  Serial.print("Channel - ");
  Serial.println(radio.getChannel());
  Serial.print("Data Rate - ");
  Serial.println(radio.getDataRate());
  Serial.println("Ready");

  setAllPixels(0,255,0);
  ring.show();
  delay(500);
  setAllPixels(0,0,0);
  ring.show();
  delay(500);
  setAllPixels(0,255,0);
  ring.show();
  delay(500);
  setAllPixels(0,0,0);
  ring.show();

}

//=============

void loop() {
    getData();
    if(stateChanged){
      
      if(!lampIsOn){
          setAllPixels(0,0,0);
          Timer1.stop();
      }
      else{
          setAllPixels(random(0,255),random(0,255),random(0,255));
          Timer1.setPeriod(personalityTimerInterval);
      }
          
      ring.show();
      stateChanged = false;
    }

    
}

//==============

void getData() {
    if ( radio.available() ) {
        radio.read( &dataReceived, sizeof(dataReceived) );
        
        showData();
        
        if(strcmp(dataReceived,"start") == 0 && currentState == 0){
          setPersonality();
          lampIsOn = true;
          stateChanged = true;
          currentState = 1;
        }
        else if(strcmp(dataReceived,"stop-") == 0 && currentState == 1){
          lampIsOn = false;
          stateChanged = true;
          currentState = 0;
        } 
           
    }
            delay(random(1,500));
        //repeatMessage(dataReceived);
}

void repeatMessage(char message[]){
  radio.stopListening();
  radio.openWritingPipe(thisSlaveAddress);
  send(message, sizeof(message));
  radio.startListening();
}

void send(char message[], int messageSize) { 
    bool rslt;
    rslt = radio.write( message, messageSize );
        // Always use sizeof() as it gives the size as the number of bytes.
        // For example if dataToSend was an int sizeof() would correctly return 2

    Serial.print("Data Sent: ");
    Serial.print(message);
    Serial.print(" (");
    Serial.print(messageSize);
    Serial.print(" bytes)");
    if (rslt) {
        Serial.println("  Acknowledge received");
    }
    else {
        Serial.println("  Tx failed");
    }
}

void showData() {
        Serial.print("Data received ");
        Serial.println(dataReceived);
        newData = false;
}


void setAllPixels(uint8_t r, uint8_t g, uint8_t b){
  for(int i=0; i<ringSize; i=i+2){
    ring.setPixelColor(i,r,g,b);
  }
  return;
}
