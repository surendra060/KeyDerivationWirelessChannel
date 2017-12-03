// **********************************************************************************
// @ Surendra Sharma - Software Code for implemntation of Quantization extraction of 
// bit streams using ---- (Tope et al. qunatizer - Reference - "Unconditionally secure 
// communications over fading channels", in IEEE Military Communications Conference 
// (MILCOM 2001), vol. 1, pp. 54–58, 2001).
// 
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
#define MAX 100
#define DISTARRAY MAX/2

RFM12B radio;

byte nodeID = 20;	//ID 10 for Transmitter Node - Need to be exchanged for Responder Node
byte pairID = 10;	//ID 20 for Responder Node - Need to be exchanged for Responder Node
uint16_t counter;	
int8_t	rxRSSI;

// Variables for Statistical (Mean/Median) and Bit-Extraction Functions
int8_t arrayCount = 0;
int8_t tempCount = 0;
int sum=0;
float avg, median; //added
float leftThresh, rightThresh; 
float leftRange, rightRange, totalRange; 


//Message Structure 
typedef struct {
  int          nodeId; //store this nodeId
  uint16_t     seqNum;    // current sequence number
  int8_t	   dataRSSI;  // Payload data for Nodes measured RSSI
  char* msg = "THE MESSAGE";
} Payload;

Payload theData;


// Arrays for storing data for calcyulation and BitExtraction - size limited by Total SRAM Size
int8_t RXrssiValues[MAX];
int8_t TXrssiValues[MAX];
int8_t tempArray[MAX];


//Setup of JeeNode on Booting
void setup(){
  Serial.begin(57600);
  counter = 0;
  rxRSSI =-100;

  //radio present check
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
  	Serial.println(F("RFM12B Detected OK!"));
  else
  	Serial.println(F("RFM12B Detection FAIL!"));
  	
  //init rssi measurement - pin and idle voltage
  radio.SetRSSI( 0, 450);
  //initialize radio (node id, freq, group id)
  radio.Initialize(nodeID, FREQUENCY, 200);
}

// Function to Blink LED
void blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}



// function for Responder node Bob - to receive message and respond/echo message
int receiveRSSI(){
  
  if (radio.ReceiveComplete()){
    if (radio.CRCPass()){

     rxRSSI = radio.ReadARSSI(BOARD_VOLTAGE); // Store the measured RSS value from Analog Pin
     
      byte thisNodeID = radio.GetSender();
      if(thisNodeID != pairID){
        
        return 0; // If Message is not correct PairID
      }
      if (*radio.DataLen != sizeof(Payload)){
        return 0;		// If Message is not correct Size
      }
          
      theData = *(Payload*)radio.Data; // read Payload data to structure elements
     
     // Save the Measured and Recived RSS values in respective Arrays
     RXrssiValues[theData.seqNum] = rxRSSI;
	 TXrssiValues[theData.seqNum] = theData.dataRSSI;
      
      //update the message elements with values to be sent		
      theData.nodeId = nodeID;
      theData.dataRSSI = rxRSSI;
      
      //echo same message back (with its own NodeID, measured RSS value in data field and same Counter)
      radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);
      
      if(theData.seqNum >= MAX -1){  // For bit extration when Array is full
        return -1;
      }
    }
					
  }
  return 0;
}


//function for Transmitetr node Alice
int sendRSSI(){
  
  if(counter == MAX){ // For bit extration when Array is full
    return -1;
  }
  
  theData.nodeId = nodeID;
  theData.seqNum = counter;
  theData.dataRSSI = rxRSSI;
  // send New or Repeat message (same counter) Message 
  radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);
		
  for(int i = 0; i < 100; i++){
    if (radio.ReceiveComplete()){
      if (radio.CRCPass()){

		rxRSSI = radio.ReadARSSI(BOARD_VOLTAGE);

        byte thisNodeID = radio.GetSender();
        if(thisNodeID != pairID){
           continue;  //Incorrect addressed message
        }
        if (*radio.DataLen != sizeof(Payload)){
          continue;   //Invalid Size message
        }
        
        theData = *(Payload*)radio.Data; // radio.DATA actually contains the struct defined above
        
        // Save the Measured and Recived RSS values in respective Arrays
        RXrssiValues[theData.seqNum] = theData.dataRSSI;
		TXrssiValues[counter] = rxRSSI;
			   
	    if (theData.seqNum == counter)  // Update counter Sequence number of Message
        	counter++;
        	
        srand(counter); // update Seed
        delay(100 + rand()% 200);  //random delay before sending a new message
        return 0;
    }
  }
  
 delay(10);  //fixed delay for a repeated message (if correct response is not received)
 }
 return 0;
}

// Function to return Array with diffreences in RSS values at a distance of distArray
void getDiffArray(int8_t out[], int8_t in[], int max, uint16_t distArray){

uint16_t i;

	for (i=0;i<MAX;i++){
	
		if (i<=(MAX - distArray-1))
			out[i] = in[i] - in[i+distArray];
		else if (i>(MAX - distArray-1))		//wraparound
			out[i] = in[i] - in[distArray - (MAX - i)];
	}
	
}


// Function to calculate Mean, SD and implment Mean based quantizations to extract bit stream
void getAvgSD_Bits(int8_t x[], int8_t n, float* avg, float* sd, uint8_t* len, float lFactor, float rFactor) {
    int16_t sum=0; float var=0; uint16_t j=0;
    // Calculate Sum, Avg, Var and Std Dev
    for(int i=0; i<n; i++) {
        sum += x[i];
    }
    *avg =  ((float) sum / (float) n);
 			
    for(int i=0; i<n; i++) {
      //  var += pow(((float) x[i] - *avg),2);
      var += (((float) x[i] - *avg) * ((float) x[i] - *avg));
    }
    
 	var = ( var / (float) n);
    *sd = (float) sqrt(var);
    
    //calculate Left and Right thresholds (As per Algorithm)
   
   
   leftThresh = ( *avg - lFactor*(*sd) );
   rightThresh = ( *avg + rFactor*(*sd) );
    
 /*   
    leftThresh = ( *avg - (0.5*(*sd)) );
	rightThresh = ( *avg + (0.5*(*sd)) );
 */  
   
Serial.print("  Avg : "); Serial.print(*avg); Serial.print("  Var : "); Serial.print((float) var); Serial.print("  Std Dev : "); Serial.print(*sd);  Serial.print(" Thresholds -> LT : "); Serial.print(leftThresh); Serial.print(" RT : "); Serial.println(rightThresh);
   
   // Qunatization based on threshold to extract bits and store in same input array (to save space)
   for(int i=0; i<n; i++) {
        if((float) x[i] < (leftThresh) ){
        	x[j] = 0; 
        	j++ ;
        }
        else if ((float) x[i] > (rightThresh) ){
				      x[j] = 1; 
				      j++ ;
        }
    }
 
    *len = j;
    
    Serial.print("Extrac Bits(Tope) (");
    Serial.print(*len);
    Serial.print(") : ");
    
	for (j=0; j < *len; j++) {  // print extracted bits
        Serial.print(x[j]);
        Serial.print(" ");
    }
    
    Serial.println(" ##");
 }


// Function to process Array data when Array is filled after MAX transmission/responses
int processArrayData(){
	//initialize variables
	int8_t minRSS, maxRSS;
	uint8_t len;
	
	float alpha_left = 0.5;
	float alpha_right = 0.5;
	
	float median =0.0;
	float avg =0.0;
	float stdDev = 0.0;
	
	// Data print for Tranmitter Node
	if(nodeID < 15)
    	Serial.print(nodeID);
  	else
    	Serial.print(pairID);
	Serial.print(" : ");
	
	// Print the RSS measurment Values
  for(uint16_t i = 0; i < MAX; i++){
    Serial.print(TXrssiValues[i]);
    Serial.print(";");
  }
  Serial.println("&&");
  
  memset(tempArray,0,MAX);
  getDiffArray(tempArray,TXrssiValues,MAX, DISTARRAY);  //tempArray of RSS differences of DISTARRAY away values (with wraparound)
  getAvgSD_Bits(tempArray, MAX, &avg, &stdDev, &len, alpha_left, alpha_right);
  
/*  memset(tempArray,0,MAX);
  getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,TXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len, alpha_left, alpha_right);
*/
  // Data print for Reponder Node 
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
  
  memset(tempArray,0,MAX);
  getDiffArray(tempArray,RXrssiValues,MAX, DISTARRAY); ////tempArray of RSS differences of DISTARRAY away values (with wraparound)
  getAvgSD_Bits(tempArray,MAX,&avg, &stdDev, &len, alpha_left, alpha_right);

/*  
  memcpy(tempArray,RXrssiValues,MAX);
  getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,RXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len, alpha_left, alpha_right);
*/   
   Serial.flush(); 
  
    delay(1000);
    counter=0;
    return 0;
}



// Infinite Loop for JeeNodes to measure, store and Process RSS values
void loop(){

  byte result = 0;
 
 if(nodeID < 15){		//If transmitter Node
    result = sendRSSI();
  } else {				//If Responder Node
    result = receiveRSSI();
  }
 
  blink(9,10); // Indicate Transmission / reception

  if( result != 0){
     processArrayData();
  }
}
