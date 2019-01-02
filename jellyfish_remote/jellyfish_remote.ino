

// SimpleTx - the master or the transmitter

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


#define CE_PIN  8
#define CSN_PIN 7

const byte slaveAddress[5] = {'R','x','A','A','A'};
char startCommand[6] = "start";
char stopCommand[6] = "stop-";

RF24 radio(CE_PIN, CSN_PIN); // Create a Radio

const int startButtonPin = 2;
const int stopButtonPin = 3;

unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 500;

void startButtonPress(){
  if ((millis() - lastDebounceTime) > debounceDelay){
    lastDebounceTime = millis();
    for(int i=0; i<100; i++){
      send(startCommand, sizeof(startCommand));
    }
  }
}

void stopButtonPress(){
  if ((millis() - lastDebounceTime) > debounceDelay){
    lastDebounceTime = millis();
    for(int i=0; i<100; i++){    
      send(stopCommand, sizeof(stopCommand));
    }
  }
}

void setup() {
    Serial.begin(9600);

    Serial.println("Starting");

    radio.begin();
    delay(1000);
    radio.stopListening();
    radio.setAutoAck(false);
    //radio.setDataRate( RF24_250KBPS );
    //radio.setRetries(10,15);

    lastDebounceTime = millis();
    
    radio.openWritingPipe(slaveAddress);
    radio.setChannel(111);
    radio.setPALevel(RF24_PA_HIGH);
    Serial.print("Channel - ");
    Serial.println(radio.getChannel());
    Serial.print("Data Rate - ");
    Serial.println(radio.getDataRate());
    pinMode(startButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(startButtonPin), startButtonPress, FALLING);

    pinMode(stopButtonPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(stopButtonPin), stopButtonPress, FALLING);
}

//====================

void loop() {
  
  //send(startCommand, sizeof(startCommand));
  delay(5000);
  //send(stopCommand, sizeof(stopCommand));
  delay(5000);  
}

//====================

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

//================


