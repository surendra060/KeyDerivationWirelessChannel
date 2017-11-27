#include <Arduino.h>
#include "RFM12B_arssi.h"
//#include <EEPROM.h>


#define FREQUENCY RF12_868MHZ

#define EEPROM_NODE_ID_LOCATION    0

//radio module pins
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin

//jeenodes run on 3.3 V, this value is in mV
#define BOARD_VOLTAGE 3300
RFM12B radio;

byte nodeA = 1;
byte nodeB = 2;
byte nodeID;
byte pairID;
boolean sniffer = false;

uint16_t counter;

typedef struct {
  int           nodeId; //store this nodeId
  uint16_t      seqNum;    // current sequence number

} Payload;

Payload theData;

void setup(){
  Serial.begin(57600);
  counter = 0;
  
  //nodeID = EEPROM.write(EEPROM_NODE_ID_LOCATION, 1);
  //nodeID = EEPROM.write(EEPROM_NODE_ID_LOCATION, 2);
 //// nodeID = EEPROM.read(EEPROM_NODE_ID_LOCATION);
  //nodeID = EEPROM.write(EEPROM_NODE_ID_LOCATION);
  //init rssi measurement - pin and iddle voltage
  radio.SetRSSI( 0, 450);

  if(nodeID == nodeA){
    pairID = nodeB;
  }
  if(nodeID == nodeB){
    pairID = nodeA;
  }

  switch(nodeID){
   case 5  :
   case 6  :
   case 10 : nodeID = nodeB;
	     sniffer = true;
             break;
   case 7  : 
   case 8  : 
   case 9  : nodeID = nodeA;
	     sniffer = true;
             break;
  }
  //init radio (node id, freq., group id)
  //if(nodeID == nodeA || nodeID == nodeB){
    radio.Initialize(nodeID, FREQUENCY, 200);
  //} else {
  //  radio.Initialize(0, FREQUENCY, 200);
  //}
}

void displayRSSI(int8_t rssi){
  Serial.print(F("\nRSSI "));

  if (rssi == RF12_ARSSI_DISABLED )
  Serial.print(F("disabled")); // ARSSI was not enabled on sketch
  else if (rssi == RF12_ARSSI_BAD_IDLE )
  Serial.print(F("has bad idle settings")); // Vidle for ARSSI has incorrect value
  else if (rssi == RF12_ARSSI_RECV )
  Serial.print(F("gateway RF reception in progress")); // can't get value another packet is in reception
  else if (rssi == RF12_ARSSI_ABOVE_MAX )
  // Value above max limit, may be set up vidle is wrong
  Serial.print(F("above maximum limit (measure and set vidle on gateway sketch"));
  else if (rssi == RF12_ARSSI_BELOW_MIN )
  // Value below min limit, may be set up vidle is wrong
  Serial.print(F("below minimum limit (measure and set vidle on gateway sketch"));
  else if (rssi == RF12_ARSSI_NB_BYTES )
  {
    // We did not sent enough byte received to for accurate RSSI calculation
    Serial.print(F("not enough bytes ("));
    Serial.print(-(rssi-RF12_ARSSI_NB_BYTES));
    Serial.print(F(") sent to gateway to have accurate ARSSI"));
  }
  else
  {
    // all sounds good, display
    // display bargraph
    Serial.print(F("["));

    for (int k=RF12_ARSSI_MIN; k<=RF12_ARSSI_MAX; k++)
    Serial.print(rssi>=k ? '=' : ' ');

    Serial.print(F("] "));

    Serial.print(rssi);
    Serial.print(F(" dB"));
  }

}

void blink(byte PIN, int DELAY_MS){
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}

void printRSSI(int8_t rssi, uint16_t counter){
  Serial.print(rssi);
  Serial.print(",");
  Serial.print(counter);
  Serial.print(";");
}

//function for responder node
void receiveRSSI(){
  if (radio.ReceiveComplete()){
    if (radio.CRCPass()){
      int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
      if (*radio.DataLen != sizeof(Payload)){
        return;
      }
      theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else

      printRSSI(rssi, theData.seqNum);
      //send same data back
      theData.nodeId = nodeID;
      radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);
    }
  }
}

void sniffRSSI(){
  if (radio.ReceiveComplete()){
    if (radio.CRCPass()){
      int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
      if (*radio.DataLen != sizeof(Payload)){
        return;
      }
      theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else

      printRSSI(rssi, theData.seqNum);
      //send same data back
    }
  }
}


//function for initiating node
void sendRSSI(){
  theData.nodeId = nodeID;
  theData.seqNum = counter;
  radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);

  for(int i = 0; i < 10; i++){
    if (radio.ReceiveComplete()){
      if (radio.CRCPass()){
        int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);

        if (*radio.DataLen != sizeof(Payload)){
          continue;
        }
        theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else
        //TODO print rssi

        printRSSI(rssi, theData.seqNum);
        counter++;
        return;
      }
    }
    delay(10);
  }
}

void loop(){
  if(sniffer){
    sniffRSSI();
  }else{
    if(nodeID == nodeA ){
      sendRSSI();
      //wait 1sec
      delay(1000);
    } else  if (nodeID == nodeB){
      receiveRSSI();
    }
  }    
  blink(9,10);

}

