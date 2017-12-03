// **********************************************************************************
// @ Surendra Sharma - Software Code for implemntation of Quantization extraction of 
// bit streams using ---- (Aono et al. qunatizer - Reference -Wireless secret key 
// generation exploiting reactance-domain scalar response of multipath fading channels",
// in IEEE Transactions on Antennas and Propagation, vol. 53, No. 11, 2005).
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
#define MAX 100     // maximal Array Size to store RSS value and process in one Window Size
RFM12B radio;

byte nodeID = 10;	//ID 10 for Transmitter Node - Need to be exchanged for Responder Node
byte pairID = 20;	//ID 20 for Responder Node - Need to be exchanged for Responder Node
uint16_t counter;	
int8_t	rxRSSI;

// Variables for Statistical (Mean/Median) and Bit-Extraction Functions
int8_t arrayCount = 0;
int8_t tempCount = 0;
float median; //added
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


// Function to calculate Median and Range values
void getMedian(int8_t x[], int8_t n, float* med, int8_t* minRSS, int8_t* maxRSS) {
    int temp;
    uint16_t i=0; uint16_t j=0;
    
    // the following two loops sort the array x in ascending order - in place sorting to savespace
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] > x[i]) {
             // swap elements if not in order
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

	*minRSS = x[MAX-6]; *maxRSS = x[5];  // Range values excluding noisy Outliers
	
	Serial.print(" Range (MinRSS - MaxRSS) : "); 
	Serial.print(*minRSS); Serial.print(" <-> "); 
	Serial.print(*maxRSS);
				 
    if(n%2==0) {
        // if there are even number of elements, return mean of the two elements in the middle
        Serial.print(" Median : "); Serial.print((float)(x[n/2] + x[n/2 - 1]) / 2.0);
        
        *med =  ((float) (x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        
        Serial.print(" Median : "); Serial.print(x[n/2]);
        *med = (float) x[n/2];
     }   
        
 }   
  
  // Function to implment Median based quantization to extract bit stream
  void getMedBits(int8_t x[], int8_t n, float* med, int8_t* minRSS, int8_t* maxRSS, uint8_t* len, float lFactor, float rFactor) {
        
   uint8_t j=0;
   //calculate Left and Right thresholds (As per Algorithm)
   leftRange = ( *med - (float) *minRSS );
   rightRange = ((float) *maxRSS - *med);
   totalRange = leftRange + rightRange;
   
   leftThresh = ( *med - leftRange*lFactor );
   rightThresh = ( *med + rightRange*rFactor );
	
   Serial.print(" LT : "); 
   Serial.print(leftThresh);
   Serial.print(" RT : "); 
   Serial.println(rightThresh);
   
   // Qunatization based on threshold to extract bits and store in same input array (to save space)
         for(uint8_t i=0; i<n; i++) {
		    if((float) x[i] < ( leftThresh )){  
				x[j] = 0; 
				j++ ;
			}
			else if ((float) x[i] > ( rightThresh )){
				x[j] = 1; 
				j++ ;
			}
        }
       
    *len = j;
    Serial.print("Extrac Bits(Aono) (");
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
	
	float alpha_left = 0.25;
	float alpha_right = 0.5;
	
	float median =0.0;
		
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
  memcpy(tempArray,TXrssiValues,MAX);  // Copy and process TXrssiValues in temp Array
  getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,TXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len, alpha_left, alpha_right);

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
  memcpy(tempArray,RXrssiValues,MAX); // Copy and process RXrssiValues in temp Array
  getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,RXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len, alpha_left, alpha_right);
   
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
