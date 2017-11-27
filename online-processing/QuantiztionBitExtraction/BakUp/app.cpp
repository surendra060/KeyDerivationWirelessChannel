#include <Arduino.h>
#include "RFM12B_arssi.h"
#include "common.h"

//radio module pins
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin

//jeenodes run on 3.3 V, this value is in mV
#define BOARD_VOLTAGE 3300

RFM12B radio;

byte nodeID = 10;
byte pairID = 20;
int counter;

void setup(){
  Serial.begin(57600);
  counter = 0;

  //radio present check
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
  Serial.println(F("RFM12B Detected OK!"));
  else
  Serial.println(F("RFM12B Detection FAIL!"));

  //init rssi measurement - pin and iddle voltage
  radio.SetRSSI( 0, 450);

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
    Serial.print(rssi>=k ? '=' : ' ');

    Serial.print(F("] "));

    Serial.print(rssi);
    Serial.print(F(" dB"));
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
      Serial.println("msg received");
      int8_t rssi = radio.ReadARSSI(BOARD_VOLTAGE);
      //byte theNodeID = radio.GetSender();

      displayRSSI(rssi);
      //Serial.println(rssi);
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
