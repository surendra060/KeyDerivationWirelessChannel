#include "RFM12B_arssi.h"
#include "common.h"
#include <math.h>
//#include <memory.h>

//radio module pins
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin

//jeenodes run on 3.3 V, this value is in mV
#define BOARD_VOLTAGE 3300
#define MAX 100
RFM12B radio;

byte nodeID = 10;
byte pairID = 20;
uint16_t counter;
int8_t		rxRSSI;

// Addition for BitExtraction Functions
int8_t arrayCount = 0;
int8_t tempCount = 0;
int sum=0;
float avg, median; //added

typedef struct {
  int          nodeId; //store this nodeId
  uint16_t     seqNum;    // current sequence number
  int8_t							dataRSSI;
  char* msg = "THE MESSAGE";

} Payload;

Payload theData;

int8_t RXrssiValues[MAX];
int8_t TXrssiValues[MAX];
int8_t tempArray[MAX];

//int8_t *RXrssiValues = (int8_t*) calloc(MAX, sizeof(int8_t));
//int8_t *TXrssiValues = (int8_t*) calloc(MAX, sizeof(int8_t));

void setup(){
  Serial.begin(57600);
  counter = 0;
  rxRSSI =-100;

  //radio present check
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
  //Serial.println(F("RFM12B Detected OK!"));
  //else
  //Serial.println(F("RFM12B Detection FAIL!"));
  //init rssi measurement - pin and idle voltage
  radio.SetRSSI( 0, 450);
  //init radio (node id, freq., group id)
  radio.Initialize(nodeID, FREQUENCY, 200);
}

void blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}

//function for responder node
int receiveRSSI(){
  //wait for msg
  //save rssi
  //respond
  //repeat

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
     // RXrssiValues[theData.seqNum] = rxRSSI;
					//	TXrssiValues[theData.seqNum] = theData.dataRSSI;
      
      //send same data back (Its own NodeID and same Counter)
    /*  
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
     	radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);
      
      if(theData.seqNum >= MAX -1){
        return -1;
      }
    }
					
  }
  return 0;
}

/*    */


//function for initiating node
int sendRSSI(){
  if(counter == MAX){
    return -1;
  }
  //send
  //wait for response
  //save rssi
  //repeat or resend
  theData.nodeId = nodeID;
  theData.seqNum = counter;
  theData.dataRSSI = rxRSSI;
  radio.Send(pairID, (const void*)(&theData), sizeof(theData), true);
		
  for(int i = 0; i < 100; i++){
    if (radio.ReceiveComplete()){
      if (radio.CRCPass()){
        //int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
																 rxRSSI = radio.ReadARSSI(BOARD_VOLTAGE);

        byte thisNodeID = radio.GetSender();
        if(thisNodeID != pairID){
          //not the messsage we are waiting for
          continue;
        }
        if (*radio.DataLen != sizeof(Payload)){
          continue;
        }
        theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else
        
        RXrssiValues[theData.seqNum] = theData.dataRSSI;
		TXrssiValues[counter] = rxRSSI;
			   
				   	
	/*		
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
        	srand(counter);
        
        delay(100 + rand()% 200);
        return 0;
    }
  }
  delay(10);
	}
	return 0;
}


void getMedian(int8_t x[], int8_t n, float* med, int8_t* minRSS, int8_t* maxRSS) {
    int temp;
    uint16_t i=0; uint16_t j=0;
    
    // the following two loops sort the array x in ascending order  //IN place sorting to savespace
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] > x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

				*minRSS = x[MAX-6]; *maxRSS = x[5];
				 Serial.print(" Range (MinRSS - MaxRSS) : "); Serial.print(*minRSS); Serial.print(" <-> "); Serial.print(*maxRSS);
				 
    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        Serial.print(" Median : "); Serial.println((float)(x[n/2] + x[n/2 - 1]) / 2.0);
        
        *med =  ((float) (x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        
        Serial.print(" Median : "); Serial.print(x[n/2]);
        *med = (float) x[n/2];
     }   
        
 }   
  
  void getMedBits(int8_t x[], int8_t n, float* med, int8_t* minRSS, int8_t* maxRSS, uint8_t* len) {
        
   uint8_t j=0;

   Serial.print(" LT : "); 
   Serial.print((int8_t) (*med - ((int8_t) *med - *minRSS )/2 ) );
   Serial.print(" RT : "); 
   Serial.println((int8_t) ( *med + (*maxRSS - (int8_t) *med)/2));
        
        for(uint8_t i=0; i<n; i++) {
				      if(x[i] < ( (int8_t) (*med - ((int8_t) *med - *minRSS )/2 ) )){  
				      			x[j] = 0; 
				      			j++ ;
				      }
				      else if (x[i] > ( (int8_t) ( *med + (*maxRSS - (int8_t) *med)/2 ) )){
								    x[j] = 1; 
								    j++ ;
				      }
        }
        
    *len = j;
    Serial.print("MedBits (");
    Serial.print(*len);
    Serial.print(") : ");
				for (j=0; j < *len; j++) {
        Serial.print(x[j]);
        Serial.print(" ");
    }
    
    Serial.println(" ##");
     
}


void getAvgSD(int8_t x[], int8_t n, float* avg, float* sd, uint8_t* len) {
    int16_t sum=0; float var=0; uint16_t j=0;
    // the following two loops sort the array x in ascending order  //IN place sorting to savespace
    for(int i=0; i<n; i++) {
        sum += x[i];
    }
    
    *avg =  ((float) sum / (float) n);
    Serial.print("  Avg : "); Serial.print(*avg);
			
    for(int i=0; i<n; i++) {
        var += pow(((float) x[i] - *avg),2);
    }
 
    *sd = (float) sqrt(var);
   Serial.print("  Var : "); Serial.print((float) var); Serial.print("  Std Dev : "); Serial.print(*sd);  Serial.print(" Comp Values L : "); Serial.print((int8_t) (*avg -(0.1*(*sd)) )); Serial.print(" Upper : "); Serial.println((int8_t) ( *avg + (0.05*(*sd))));
    
    for(int i=0; i<n; i++) {
        if(x[i] < ( (int8_t) ( *avg -(0.1*(*sd)) ))){
        			x[j] = 0; 
        			j++ ;
        }
        else if (x[i] > ( (int8_t) ( *avg + (0.05*(*sd)) ))){
				      x[j] = 1; 
				      j++ ;
        }
    }
 
    *len = j;
    Serial.print("AvgBits (");
    Serial.print(*len);
    Serial.print(") : ");
				for (j=0; j < *len; j++) {
        Serial.print(x[j]);
        Serial.print(" ");
    }
    
    Serial.println(" ##");
 }


int ExtractBitsPrint(){
	
	int8_t minRSS, maxRSS;
	uint8_t len;
	float median =0.0;
	float avg =0.0;
	float stdDev = 0.0;
	
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
  
  memset(tempArray,0,MAX);
  memcpy(tempArray,TXrssiValues,MAX);
  getAvgSD(tempArray,MAX,&avg, &stdDev, &len);
  memcpy(tempArray,TXrssiValues,MAX);
  getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,TXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len);

   /* */
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
  
  memset(tempArray,0,MAX);
  memcpy(tempArray,RXrssiValues,MAX);
  getAvgSD(tempArray,MAX,&avg, &stdDev,&len);
  memcpy(tempArray,RXrssiValues,MAX);
		getMedian(tempArray, MAX, &median, &minRSS, &maxRSS);
  memcpy(tempArray,RXrssiValues,MAX);
  getMedBits(tempArray, MAX, &median, &minRSS, &maxRSS, &len);
   
   Serial.flush(); 
  //while(true){
    delay(1000);
    counter=0;
    return 0;

  ///}
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
     ExtractBitsPrint();
  }
}
