#include <Arduino.h>
#include <RFM12B_arssi.h>
#include "common.h"

//radio module pins
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin

//jeenodes run on 3.3 V, this value is in mV
#define BOARD_VOLTAGE 3300

RFM12B radio;

byte nodeID = 10;
byte pairID = 20;

const int8_t buffSize=10;
int8_t rxArray[buffSize];
int8_t counter = 0;
int8_t arrayCount = 0;
int8_t tempCount = 0;
int sum=0;
float avg, median; //added

void setup(){
  Serial.begin(57600);
  counter = 0;

  //radio present check
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
  Serial.println(F("RFM12B Detected OK!"));
  else
  Serial.println(F("RFM12B Detection FAIL!"));

  //init rssi measurement - pin and idle voltage
  radio.SetRSSI(0, 450);

  //init radio (node id, freq., group id)
  radio.Initialize(nodeID, FREQUENCY, 200);
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
    Serial.print(rssi>=k ? '*' : ' ');

    Serial.print(F("] "));

    Serial.print(rssi);
    Serial.println(F(" dB - "));
    
    //extractBS(rssi);
  }
			
}

// added for Moving window averaging
void getWindowArray(int8_t rxstr){

				rxArray[arrayCount] = rxstr;
				arrayCount++;
				
				if (arrayCount == (buffSize)){
								arrayCount=0;
								avg=0; sum=0;
				}
}

void getBS(int8_t rxArray[], int8_t buffSize){

					 Serial.println("\n################\n Array : ");	
						for (tempCount=0; tempCount<buffSize; tempCount++){
      
    				Serial.print(rxArray[tempCount]);	
    				sum += rxArray[tempCount];
								Serial.print(F(" "));
						}	
						
				avg = sum/buffSize;
				Serial.print(F("\n Avg = ")); Serial.print(avg);	
}

// Median

float getMedian(int8_t x[], int8_t n) {
    float temp;
    int i, j;
    // the following two loops sort the array x in ascending order
    for(i=0; i<n-1; i++) {
        for(j=i+1; j<n; j++) {
            if(x[j] < x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }

    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
        return((x[n/2] + x[n/2 - 1]) / 2.0);
    } else {
        // else return the element in the middle
        return x[n/2];
    }
}

void blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,LOW);
  delay(DELAY_MS);
  digitalWrite(PIN,HIGH);
}

void loop(){

  if (radio.ReceiveComplete())
  {

    if (radio.CRCPass())
    {

      //if(radio.ACKRequested()){
      //  radio.SendACK();
      //}
      Serial.print("New Msg received on node :  ");
      Serial.print(nodeID);	
      Serial.print("  from Remote node :  ");
      Serial.print(pairID);	
      
      int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
      //byte theNodeID = radio.GetSender();

      displayRSSI(rssi);
      getWindowArray(rssi);
      if (arrayCount==0) {
      getBS(rxArray, buffSize);
      median = getMedian(rxArray, buffSize);
     
     Serial.print(F("  Median = ")); Serial.print(median);	
					Serial.print(F(" \n################\n\n "));
      
      }      
      blink(9,10);
    }
  }

  if(counter >= 100){
    counter = 0;
    char* msg = "hellohellohellohellohellohellohellohellohellohellohellohello";

    radio.Send(pairID, &msg, sizeof(msg),true);
    blink(9,10);
  }

  counter++;
  delay(10);


}
