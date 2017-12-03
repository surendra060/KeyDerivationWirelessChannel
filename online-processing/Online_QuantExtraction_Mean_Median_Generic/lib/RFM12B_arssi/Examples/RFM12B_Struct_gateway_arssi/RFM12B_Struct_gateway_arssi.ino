#include <avr/power.h>
#include <RFM12B_arssi.h>
#include <SPI.h>
#include <SPIFlash.h>

#define NODEID      1
#define NETWORKID   100
#define FREQUENCY   RF12_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin
#define RSSI_PIN    0  // Analog entry where is connected ARSSI signal
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack

RFM12B radio;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network
byte ackCount=0;

typedef struct {		
  int           nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  uint16_t      vcc;    // current Arduino VCC power 
  int16_t       temp;   // internal Arduino temperature 
} Payload;
Payload theData;

uint16_t vcc=0;
int16_t temperature=0;
volatile uint8_t  adc_irq_cnt;


/* ======================================================================
Function: Interrupt routine for ADC
Purpose : Fired when ADC interrupt occured (mainly end of convertion)
Input   : 
Output  : 
Comments: Used by readADCLowNoise
====================================================================== */
ISR(ADC_vect)  
{
  // Increment ADC sample count 
  adc_irq_cnt++;
}

/* ======================================================================
Function: readADCLowNoise
Purpose : Read Analog Value with reducing noise for more accuracy
Input   : true return the average value, false return only the sum
Output  : average value read
Comments: hard coded to read 8 samples each time
          ADMUX Channel must have been set before this call
====================================================================== */
uint16_t readADCLowNoise(bool average)
{
  uint8_t low, high;
  uint16_t sum = 0;
  
  // Start 1st Conversion, but ignore it, can be hazardous
  ADCSRA |= _BV(ADSC); 
  
  // as we will enter into deep sleep mode, flush serial to avoid
  // data loss or corrupted
  Serial.flush();

  // wait for first dummy conversion
  while (bit_is_set(ADCSRA,ADSC));

  // Initialize ADC sample counter
  adc_irq_cnt = 0;

  // Want to have an interrupt when the conversion is done
  ADCSRA |= _BV( ADIE );
  
  // Loop thru samples
  do
  {
    // Enable Noise Reduction Sleep Mode
    set_sleep_mode( SLEEP_MODE_ADC );
    sleep_enable();

    // Wait until conversion is finished 
    do
    {
      // The following line of code is only important on the second pass.  For the first pass it has no effect.
      // Ensure interrupts are enabled before sleeping
      sei();
      // Sleep (MUST be called immediately after sei)
      sleep_cpu();
      // Checking the conversion status has to be done with interrupts disabled to avoid a race condition
      // Disable interrupts so the while below is performed without interruption
      cli();
    }
    while (bit_is_set(ADCSRA,ADSC));

    // No more sleeping
    sleep_disable();
    // Enable interrupts
    sei();
    
    // read low first
    low  = ADCL;
    high = ADCH;
    
    // Sum the total
    sum += ((high << 8) | low);
    
  }
  // Hard coded to read 8 samples
  while (adc_irq_cnt<8);
  
  // No more interrupts needed for this
  ADCSRA &= ~ _BV( ADIE );
  
  // Return the average divided by 8 (8 samples) if asked
  return ( average ? sum >> 3 : sum );
  
}

/* ======================================================================
Function: readVcc
Purpose : Read and Calculate V powered, the Voltage on Arduino VCC pin
Input   : -
Output  : value readed in mV
Comments: ADC Channel input is modified
====================================================================== */
uint16_t readVcc() 
{
  uint16_t value; 
  
  // Indicate RFM12B IRQ we're doing conversion 
  // so it should not doing any RSSI acquisition in the interval
  // we're using the ADC
  radio.noRSSI(true);
  
  // Read 1.1V reference against AVcc
  // REFS1 REFS0          --> 0 1, AVcc external ref. -Selects AVcc external reference
  // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
  ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  // Take care, changing reference from VCC to 1.1V bandgap can take some time, this is due
  // to the fact that the capacitor on aref pin need to discharge or to charge 
  delay(10);  

  // read value
  value = readADCLowNoise(true);
  
  // we done with ADC
  radio.noRSSI(false);
  
  // Vcc reference in millivolts
  // can be adjusted 1100L if 1V1 reference but has tolerance of 10% so you can measure it
  // and change it there, or better use it as parameter
  return ( (( 1023L * 1100L) / value) ); 
}

/* ======================================================================
Function: readInternalTemperature
Purpose : Read and Calculate Arduino Internal Temperature
Input   : -
Output  : value readed in °C (*10) ie 210 is 21.0 °C
Comments: ADC Channel input is modified
====================================================================== */
int16_t readInternalTemperature(void)
{
  uint16_t adc_value; 
  float temp;
  
  // Indicate RFM12B IRQ we're doing conversion 
  // so it should not doing any RSSI acquisition in the interval
  // we're using the ADC
  radio.noRSSI(true);
  
  // Read 1.1V reference against AVcc
  // REFS1 REFS0          --> 1 1, 1V1 internal ref.  -Selects AVcc external reference
  // MUX3 MUX2 MUX1 MUX0  --> 1000 Temp               -Selects channel 8, internal temperature 
  ADMUX = (1<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (0<<MUX2) | (0<<MUX1) | (0<<MUX0);

  // Take care, changing reference from VCC to 1.1V can take some time, this is due
  // to the fact that the capacitor on aref pin need to discharge or to charge 
  delay(10);  

  // read value
  adc_value = readADCLowNoise(true);
  
  // we done with ADC
  radio.noRSSI(false);
  
  // The offset of 324.31 could be wrong. It is just an indication.
  temp = (adc_value - 324.31 ) / 1.22;

  // The returned temperature is in degrees Celcius.
  return ((int16_t) (temp*10));
}


/* ======================================================================
Function: setup
Purpose : Configuration of Arduino I/O and other stuff
Input   : -
Output  : -
Comments: 
====================================================================== */
void setup() {

  Serial.begin(SERIAL_BAUD);
  delay(10);
  Serial.println(F("RFM12B Gateway Sample"));
  
   // Try to detect and Init On moteino RF12 device SS is D10 and IRQ D2 (default)
  // So the parameters are optional
  // if radio.isPresent()
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
    Serial.println(F("RFM12B Detected OK!"));
  else
    Serial.println(F("RFM12B Detection FAIL! (is chip present?)"));
    
  // Ok now we indicate we want to use ARSSI reading, on my board I put ARSSI signal
  // on Analog 0 pinMode with Idle Arssi to 300 mv (need to be adjusted)
  radio.SetRSSI( RSSI_PIN, 350 );
  
  radio.Initialize(NODEID, FREQUENCY, NETWORKID);
  radio.Encrypt((byte*)KEY);
  
  Serial.print(F("Transmitting at "));
  Serial.print(FREQUENCY==RF12_433MHZ ? 433 : FREQUENCY==RF12_868MHZ ? 868 : 915);
  Serial.println(F("Mhz..."));
  
  // If RSSI is activated for this board
  // display relative information
  if ( radio.getRSSIIdle())
  {
    Serial.print(F("ARSSI Enabled\nConnect ARSSI signal on Analog pin "));
    Serial.print(radio.getRSSIAnalogPin());
    Serial.print(F(" of this board\nARSSI idle voltage is set to "));
    Serial.print(radio.getRSSIIdle());
    Serial.println(F(" mV"));
  }
  else
  {
    Serial.println(F("ARSSI Disabled for this board"));
  }
  
  if (flash.initialize())
    Serial.println(F("SPI Flash Init OK!"));
  else
    Serial.println(F("SPI Flash Init FAIL! (is chip present?)"));
  
  // read vcc value
  vcc=readVcc();
  Serial.print(F("Vcc = "));
  Serial.print(vcc);
  Serial.println(F(" mV"));

  // read temperature value
  temperature=readInternalTemperature();
  Serial.print(F("temp= "));
  Serial.print(temperature/10.0);
  Serial.println(F(" C\n"));
  
  Serial.println(F("Quick command summary, type it any time"));
  Serial.println(F("  d : dump flash"));
  Serial.println(F("  e : erase flash"));
  Serial.println(F("  i : print flash ID"));
  Serial.println(F("  v : read and display VCC"));
  Serial.println(F("  t : read and display temperature"));
  Serial.print(  F("  a : measure and display a set of ARSSI Analog Pin "));
  Serial.print(radio.getRSSIAnalogPin());
  Serial.println(F(")"));

  Serial.println(F("\nWaiting for receiving data from node ...\n"));
}

/* ======================================================================
Function: loop
Purpose : really need explanations ???
Input   : -
Output  : -
Comments: 
====================================================================== */
void loop() {
  //process any serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    
    if (input == 'd') //d=dump flash area
    {
      Serial.println(F("Flash content:"));
      int counter = 0;

      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        Serial.print('.');
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'e')
    {
      Serial.print(F("Erasing Flash chip ... "));
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      Serial.print(F("DeviceID: "));
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
    // Read and display VCC value
    if (input == 'v') 
    {
      Serial.print(F("VCC : "));
      Serial.print(readVcc());
      Serial.println(F(" mV"));
    }
    // Read and display internal temperature
    if (input == 't') 
    {
      Serial.print(F("Temp: "));
      Serial.print(readInternalTemperature()/10.0);
      Serial.println(F(" C"));
    }
    // Read physical Arssi value, can be used to 
    // determine vidle to pass to SetRSSI()
    if (input == 'a') 
    {
      int16_t adc_value;
      
      // display some ARSII raw samples
      for (uint8_t i=0; i<=10; i++)
      {
        // Indicate RFM12B IRQ we're doing conversion 
        // so it should not doing any RSSI acquisition in the interval
        // we're using the ADC
        radio.noRSSI(true);
        
        // Selects AVcc external reference and ARSSI Analog pin
        ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (RSSI_PIN);

        // Take care, changing reference from VCC to 1.1V bandgap can take some time, this is due
        // to the fact that the capacitor on aref pin need to discharge or to charge 
        delay(10);  

        // read value and average samples
        adc_value = readADCLowNoise(true);
        
        // we done with ADC
        radio.noRSSI(false);
        
        // Now display
        Serial.print(F("ARSSI : "));
        Serial.print((uint32_t) vcc * adc_value / 1024); // convert arssi value to mV
        Serial.println(F("mV"));
      }
    }
  }

  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      // Get RSSI of received packet, ACK request
      // Set parameter to VCC voltage of arduino
      bool ack_requested = radio.ACKRequested();
      byte theNodeID = radio.GetSender();
      int8_t rssi = radio.ReadARSSI(vcc);
      int8_t k;
      
      Serial.print(F("Received struct["));
      Serial.print(*radio.DataLen);
      Serial.print("] From nodeID ");
      Serial.print(theNodeID, DEC);
      
      if (*radio.DataLen != sizeof(Payload))
        Serial.print(F("Invalid payload received, not matching Payload struct!"));
      else
      {
        theData = *(Payload*)radio.Data; //assume radio.DATA actually contains our struct and not something else
        
         // Display Data info includiing RSSI value
        Serial.print(F(" with { "));
        Serial.print(F("temp="));
        Serial.print(theData.temp/10.0);
        Serial.print(F(" C  Vcc="));
        Serial.print(theData.vcc);
        Serial.print(F(" mV"));
        Serial.print(F(" uptime="));
        Serial.print(theData.uptime/1000);
        Serial.print(F(" s }"));

        // Send ACK ASAP
        // when a node requests an ACK, respond to the ACK 
        // and also later send a packet containing RSSI requesting an ACK
        if (ack_requested)
        {
          radio.SendACK();
          Serial.print(F(" ACKED"));
        }
  
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
          
          for (k=RF12_ARSSI_MIN; k<=RF12_ARSSI_MAX; k++)
            Serial.print(rssi>=k ? '=' : ' ');
            
          Serial.print(F("] "));

          Serial.print(rssi);
          Serial.print(F(" dB"));
        }    
      }
      
      // when a node requests an ACK, respond to the ACK (already done)
      // and also send a packet containing RSSI requesting an ACK
      // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
      if (ack_requested)
      {
        //radio.SendACK();
        //Serial.print(F(" ACKED"));
        Serial.print(F(" Sending back RSSI to nodeID "));
        Serial.print(theNodeID);
        Serial.flush();
        radio.Send(theNodeID, &rssi, 1, true);
        Serial.print(F(" ... waiting for ACK..."));
        if (waitForAck(theNodeID)) Serial.print(F("ok!"));
        else Serial.print(F("nothing!"));
      }
      Serial.println();
      Blink(LED,3);
    }
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

// wait a few milliseconds for proper ACK to me, return true if indeed received
static bool waitForAck(byte theNodeID) {
  long now = millis();
  while (millis() - now <= ACK_TIME) {
    if (radio.ACKReceived(theNodeID))
      return true;
  }
  return false;
}
