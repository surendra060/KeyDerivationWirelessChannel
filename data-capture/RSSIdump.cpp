// **********************************************************************************
// @ Surendra Sharma - Software Code for implemntation of Capture and Dumping of captured RSS.
// data to serial port. RSS values measured at Each Tx/Rx node are stored in an array. Each measured 
// RSS value is also sent to the pair Node so that the both sides can have both sides array for dumping
// Storing to file. 
// NOTE :- In actual implemenation these measured values are secrets and will not be 
// Send to other node as any adversary may capture it. This is just to synchronize and make capturing 
// possible / easy at  only One end /PC for both arrays for statistical analysis and bit extraction.


// RSSI values are extracted from Radio chip HopeRF RFM12B RF Module on JeeNode v5 using 
// Charles-Henri Hallard modified Library RFM12B. Library hosted on 
// https://github.com/hallard/RFM12B_arssi & details on http://hallard.me/rfm12b_arssi-library/
// 
// Original Licennse also Enclosed Below. Any modification / reuse of the software must include
// this license.
// **********************************************************************************

// **********************************************************************************
// Driver definition for HopeRF RFM12B RF Module
// **********************************************************************************
// License is the same as original libraries
//
// For any explanation see RFM12 module
// http://www.hoperf.com/rf/fsk_module/RFM12B.htm
//
// Code based on following datasheet
// http://www.hoperf.com/upload/rf/RFM12B.pdf
//
// Based on the RFM12 driver from jeelabs.com (2009-02-09 <jc@wippler.nl>)
// modified 2012-12-12 (C) felix@lowpowerlab.com from LowPowerLab.com
//
// Modified by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2014-08-01 - First release
//                              Added dynamic Chip select Pin selection
//                              Added dynamic IRQ pin selection
//                              Added check that RFM12B module is present
//                              Added true ARSSI reading
//                                    Need hardware hack depending on module type
//
// All text above must be included in any redistribution.
//
// **********************************************************************************




#include "RFM12B_arssi.h"

#define FREQUENCY RF12_868MHZ
#define SERIAL_FREQUENCY 57600

//radio module pins
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin

//jeenodes run on 3.3 V, this value is in mV
#define BOARD_VOLTAGE 3300
#define MAX 500
RFM12B radio;

byte nodeID = 10;	//ID 10 for Transmitter Node - Need to be exchanged for Responder Node
byte pairID = 20;	//ID 20 for Responder Node - Need to be exchanged for Responder Node
byte brdcastID = 0;
uint16_t counter;
int8_t		rxRSSI;


typedef struct {
  int          nodeId; // nodeId as a field of message
  uint16_t     seqNum;    // current sequence number
  int8_t							dataRSSI;
  char* msg = "THE MESSAGE";

} Payload;

Payload theData;

int8_t RXrssiValues[MAX];
int8_t TXrssiValues[MAX];


void setup(){
  Serial.begin(57600);
  counter = 0;
  rxRSSI =-100;

  //radio initialise and check
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
		radio.SetRSSI( 0, 450);
  radio.Initialize(nodeID, FREQUENCY, 200);
}

void blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}

//function for RX and Responder node : 20
int receiveRSSI(){
 
  if (radio.ReceiveComplete()){
    if (radio.CRCPass()){
     // int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
     rxRSSI = radio.ReadARSSI(BOARD_VOLTAGE);
     
      byte thisNodeID = radio.GetSender();
      if(thisNodeID != pairID){
        //not the messsage we are waiting for
        return 0;
      }
      if (*radio.DataLen != sizeof(Payload)){
        return 0;
      }
          
      theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else
           
    // send same data back (Its own NodeID and same Counter) Suppressed for File Output, 
    
    /*  Can be enabled to check on Serial Monitor of Arduino SDK
    
      	Serial.print(F(" RX_CNT : "));
        Serial.print(theData.seqNum);
        Serial.print(F(" SEQ : "));
        Serial.print(theData.seqNum);
        Serial.print(F("  dataRSSI : "));
        Serial.print(theData.dataRSSI);
        Serial.print(F("  rxRSSI : "));
        Serial.println(rxRSSI);
      		Serial.flush();
    */  		
    
     RXrssiValues[theData.seqNum] = rxRSSI;
	 TXrssiValues[theData.seqNum] = theData.dataRSSI;
      		
      theData.nodeId = nodeID;
      theData.dataRSSI = rxRSSI;
      delay(10);
     // radio.Send(pairID, (const void*)(&theData), sizeof(theData), true); 
     radio.Send(brdcastID, (const void*)(&theData), sizeof(theData), true);  // convereted to Broadcast ID
      
      if(theData.seqNum >= MAX -1){
        return -1;
      }
    }
					
  }
  return 0;
}


//function for TX node : 10
int sendRSSI(){
  if(counter == MAX){
    return -1;
  }
  //send A packet and Immediately wait for receive, If not received till time-out resend with same counter
  // If received the response measure the RSSI, Process Next Packet(with its measured val), Wait for Random delay
  //Send the next packet with increased counter
  
  theData.nodeId = nodeID;
  theData.seqNum = counter;
  theData.dataRSSI = rxRSSI;
  //radio.Send(pairID, (const void*)(&theData), sizeof(theData), true); 
	radio.Send(brdcastID, (const void*)(&theData), sizeof(theData), true);  // convereted to Broadcast 	
  for(int i = 0; i < 100; i++){
    if (radio.ReceiveComplete()){
      if (radio.CRCPass()){

		rxRSSI = radio.ReadARSSI(BOARD_VOLTAGE);
        byte thisNodeID = radio.GetSender();
        if(thisNodeID != pairID){
          //not the messsage from Its correct responder
          continue;
        }
        if (*radio.DataLen != sizeof(Payload)){
          continue;
        }
        theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else
        
        RXrssiValues[theData.seqNum] = theData.dataRSSI;
		TXrssiValues[counter] = rxRSSI;
			   
	 /*		Suppressed for File Output, Can be enabled to check on Serial Monitor of Arduino SDK
	 
   	    Serial.print(F(" TX_CNT : ")); 
        Serial.print(counter);
        Serial.print(F(" SEQ : "));
        Serial.print(theData.seqNum);
        Serial.print(F("  dataRSSI : "));
        Serial.print(theData.dataRSSI);
        Serial.print(F("  rxRSSI : "));
        Serial.println(rxRSSI);
        Serial.flush();
     /* */  
        if (theData.seqNum == counter) 
        	counter++;
        	srand(counter*abs(rxRSSI));
        
        delay(100 + rand()% 200);
        return 0;
    }
  }
  delay(50);
}
	return 0;
}

int serailDumpRSSI(){
		
	//Serial.print("TX ");
	
	if(nodeID < 15)
    Serial.print(nodeID);
  else 
    Serial.print(pairID);
	
		Serial.print(" : ");
	
	
  for(uint16_t i = 0; i < MAX; i++){
    Serial.print(TXrssiValues[i]);
    Serial.print(";");
  }
  Serial.println("&&");
 
   Serial.flush(); 
 
 //Serial.print("RX ");
 
 	if(nodeID < 15)
    Serial.print(pairID);
  else 
    Serial.print(nodeID);
	
		Serial.print(" : ");

  for(uint16_t i = 0; i < MAX; i++){
    Serial.print(RXrssiValues[i]);
    Serial.print(";");
  }
  Serial.println("&&");
  
  Serial.flush(); 
  
   delay(1000);
    counter=0;
    return 0;
}


void loop(){

  byte result = 0;
 
 if(nodeID < 15){
    result = sendRSSI();
  } else {
    result = receiveRSSI();
  }
 /*   */
  blink(9,10);
  //Serial.println(result);
  if( result != 0){
     serailDumpRSSI();
  }
}
