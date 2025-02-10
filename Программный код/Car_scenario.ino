#include <SPI.h>
#include "printf.h"
#include "RF24.h"



#define CE_PIN 6
#define CSN_PIN 7
RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = { "1Node", "2Node" };

// *** HERE!!! HERE!!! HERE!!! ***

bool radioNumber = 1;  // 0 uses address[0] to transmit, 1 uses address[1] to transmit


const char msg_ping[] = "ready!";
const char msg_pong[] = "Bye!";

bool role = radioNumber==1;  // true = TX role, false = RX role
float payload = 0.0;

const int motor1In1 = 2;
const int motor1In2 = 3;
const int motor2In1 = 4;
const int motor2In2 = 5;

const int US_MODE = 0;
const int IR_MODE = 1;

const int LEAVING = 1;
const int PARKING = 0;
const int WAITING = 2;

int currentState = WAITING;

const int FORWARD = 0;
const int BACKWARD = 1;
const int STOP = 2;

bool gone = false;


bool meListening = !role;

int echoPin = 9;
int trigPin = 8;
int desiredDistance = 5;

void moveCar(int mode) {
  if (mode != STOP){
    digitalWrite(motor1In1, mode == 1);
    digitalWrite(motor1In2, mode == 0);  
    digitalWrite(motor2In1, mode == 0);
    digitalWrite(motor2In2, mode == 1);    
  }
  else{
    digitalWrite(motor1In1, 0);
    digitalWrite(motor1In2, 0);  
    digitalWrite(motor2In1, 0);
    digitalWrite(motor2In2, 0);    
  }
 
}

bool radioTalk(bool myRole){
  if (role) {
    // This device is a TX node
    if (meListening){
      radio.stopListening();
      meListening = false;
      delay(20);
    }
    
    unsigned long start_timer = micros();                // start the timer
    //bool report = radio.write(&payload, sizeof(float));  // transmit & save the report
    bool report = radio.write(&msg_ping, sizeof(msg_ping));  // transmit & save the report
    unsigned long end_timer = micros();                  // end the timer

    if (report) {
      Serial.print(F("Transmission successful! "));  // payload was delivered
      Serial.print(F("Time to transmit = "));
      Serial.print(end_timer - start_timer);  // print the timer result
      Serial.print(F(" us. Sent: "));
      Serial.println(msg_ping);  // print payload sent
      //Serial.println(payload);  // print payload sent
      //payload += 0.01;          // increment float payload
    } else {
      Serial.println(F("Transmission failed or timed out"));  // payload was not delivered
    }
    // to make this example readable in the serial monitor
    delay(1600);  // slow transmissions down by 1 second

    //waiting for the answer

    radio.startListening();
    meListening = true;
    unsigned long started_waiting_at = micros();            
    boolean timeout = false;                                   
    while ( ! radio.available() ){
      Serial.println("I am still waiting...");
      delay(100);
    }

    char responce[32] = "";
    radio.read(&responce, sizeof(responce));
    Serial.print(F("Sent "));
    Serial.print(F(", Got response "));
    Serial.print(responce);
    if (responce == msg_pong){
      return true;
    }
    else{
      return false;
    }
    

  } else {
    // This device is a RX node

    if (!meListening){
      if(gone){
        radio.write( &msg_pong, sizeof(msg_pong) );               
        Serial.print(F("Sent response "));
        Serial.println(msg_pong);
        radio.startListening();
        meListening = true;
        delay(20);
        return true;
      }
      else{
        return false;
      }
    }
    else{
      uint8_t pipe;
      char buffer[32] = "";
      if (radio.available(&pipe)) {              // is there a payload? get the pipe number that received it
        uint8_t bytes = radio.getPayloadSize();  // get the size of the payload
        radio.read(&buffer, bytes);             // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes);  // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe);  // print the pipe number
        Serial.print(F(": "));
        //Serial.println(payload);  // print the payload's value
        Serial.println(buffer);  // print the payload's value  
        
        if (buffer == msg_ping){
          Serial.println("GOOD!");
          radio.stopListening();
          meListening = false; 
          gone = false;
          return true;
       
        }
        else{
          return false;
        }   
      } 
    }
    
  }  // role
} //radioTalk

int measure_cm(){
  long duration;
  int cm;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  cm = duration / 58;
  return cm;
} // measure_cm

bool checkDist(int mode){
  if (mode == IR_MODE){
    int IR_TRESHOLD = 300;
    return (analogRead(A0) <= IR_TRESHOLD);
  } else if (mode == US_MODE){
    int US_TRESHOLD = desiredDistance;
    return (measure_cm() <= US_TRESHOLD);
  }
} //checkDist


bool checkDistFar(){
  int US_TRESHOLD = 3 * desiredDistance + 3;
  return (measure_cm() >= US_TRESHOLD);
}
void setup() {

  Serial.begin(115200);
  while (!Serial) {
    // some boards need to wait to ensure access to serial over USB
  }


  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  
  pinMode(motor1In1, OUTPUT);
  pinMode(motor1In2, OUTPUT);
  pinMode(motor2In1, OUTPUT);
  pinMode(motor2In2, OUTPUT);
  moveCar(STOP);

  

  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {}  // hold in infinite loop
  }

  // print example's introductory prompt
  Serial.println(F("RF24/examples/GettingStarted"));

  // To set the radioNumber via the Serial monitor on startup
  //Serial.println(F("Which radio is this? Enter '0' or '1'. Defaults to '0'"));
  
  Serial.print(F("radioNumber = "));
  Serial.println((int)radioNumber);

  // role variable is hardcoded to RX behavior, inform the user of this
  Serial.println(F("*** PRESS 'T' to begin transmitting to the other node"));

  // Set the PA Level low to try preventing power supply related problems
  // because these examples are likely run with nodes in close proximity to
  // each other.
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.

  // save on transmission time by setting the radio to only transmit the
  // number of bytes we need to transmit a float
  radio.setPayloadSize(sizeof(msg_ping));  // float datatype occupies 4 bytes

  // set the TX address of the RX node into the TX pipe
  radio.openWritingPipe(address[radioNumber]);  // always uses pipe 0
  // set the RX address of the TX node into a RX pipe
  radio.openReadingPipe(1, address[!radioNumber]);  // using pipe 1

  if (role) {
    radio.stopListening();  // put radio in TX mode
  } else {
    radio.startListening();  // put radio in RX mode
  }

}  // setup

void loop() {
  radioTalk(role);
  /*
  
  if (role){
    if (Serial.available()){
      String data = Serial.readString();
      if (data == "chop-chop!"){
        while(radioTalk(role)){
          Serial.println("Not yet...");
          delay(100);         
        }
        Serial.println("CAROUSEL!");
      }
    }
    
    
  }
  else{
    if (meListening){
      if (radioTalk(role)){
        //Serial.println()
        if (currentState == WAITING){
          if (checkDist(US_MODE)){
            currentState = LEAVING;
          }
          else{
            currentState = PARKING;
          }
          gone = false;
        } 
        else{
          currentState = WAITING;
        }
      }
      else{
        currentState = WAITING;
      }
    }
    else{
      if (currentState == LEAVING)
      {
        if (checkDistFar()){
          gone = true;
          currentState = WAITING;
          radioTalk(role);
        }
        else{
          gone = false;
        }
      }
      else if(currentState == PARKING)
      {
        if (checkDist(US_MODE))
        {
          gone = true;
          currentState = WAITING;
          radioTalk(role);
        }
        else{
          gone = false;
        } 
      }
      else{
        Serial.println("SOMETHING IS SO WRONG WITH MY WAITING!!!");
      }
      
    }
  }

  moveCar(currentState);
*/
  

  
/*
  if (Serial.available()) {
    // change the role via the serial monitor

    char c = toupper(Serial.read());
    if (c == 'T' && !role) {
      // Become the TX node

      role = true;
      Serial.println(F("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK"));
      radio.stopListening();

    } else if (c == 'R' && role) {
      // Become the RX node

      role = false;
      Serial.println(F("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK"));
      radio.startListening();
    }
  }
*/
}  // loop
