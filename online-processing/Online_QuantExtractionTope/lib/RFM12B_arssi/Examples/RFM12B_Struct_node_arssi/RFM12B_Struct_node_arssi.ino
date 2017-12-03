#include <RFM12B_arssi.h>
#include <SPI.h>
#include <SPIFlash.h>

#define NODEID      99
#define NETWORKID   100
#define GATEWAYID   1
#define FREQUENCY   RF12_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define KEY         "thisIsEncryptKey" //has to be same 16 characters/bytes on all nodes, not more not less!
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin
#define LED         9
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack

int TRANSMITPERIOD = 1000; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM12B radio;
long lastPeriod = -1;

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
  // will check after wake up
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
  
  Serial.println(F("RFM12B Node Sample"));

  
  // Try to detect and Init On moteino RF12 device SS is D10 and IRQ D2 (default)
  // So the parameters are optional
  // if radio.isPresent()
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
    Serial.println(F("RFM12B Detected OK!"));
  else
    Serial.println(F("RFM12B Detection FAIL! (is chip present?)"));
  
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
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

  Serial.println(F("\nStarting to send data to gateway ...\n"));
  
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
    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print(F("\nChanging delay to "));
      Serial.print(TRANSMITPERIOD);
      Serial.println(F("ms\n"));
    }
    
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
      Serial.println(F("mV"));
    }
    // Read and display internal temperature
    if (input == 't') 
    {
      Serial.print(F("Temp: "));
      Serial.print(readInternalTemperature()/10);
      Serial.println(F("C"));
    }
  }

  //check for any received packets
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      int8_t rssi, k;
      
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      
      
      // len should be 1 (RSSI byte)
      if (*radio.DataLen != 1)
        Serial.print(F("Invalid RSSI payload received, not matching RSSI ACK Format!"));
      else
        rssi = radio.Data[0]; //radio.DATA actually contains RSSI

      Serial.print(F("RSSI "));
      
      
      if (rssi == RF12_ARSSI_DISABLED )
        Serial.print(F("disabled on gateway")); // ARSSI was not enabled on Gateway sketch
      else if (rssi == RF12_ARSSI_BAD_IDLE )
        Serial.print(F("has bad idle settings")); // Vidle for ARSSI has incorrect value on Gateway sketch
      else if (rssi == RF12_ARSSI_RECV )
        Serial.print(F("gateway RF reception in progress")); // can't get value another packet is in reception on GW
      else if (rssi == RF12_ARSSI_ABOVE_MAX )
        // Value above max limit, may be set up vidle is wrong on Gateway
        Serial.print(F("above maximum limit (measure and set vidle on gateway sketch"));
      else if (rssi == RF12_ARSSI_BELOW_MIN )
        // Value below min limit, may be set up vidle is wrong on Gateway
        Serial.print(F("below minimum limit (measure and set vidle on gateway sketch"));
      else if (rssi == RF12_ARSSI_NB_BYTES )
      {
        // We did not sent enough byte to gateway for accurate RSSI calculation
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


      if (radio.ACKRequested())
      {
        radio.SendACK();
        Serial.print(F(" ACKED"));
      }
      Serial.println();
  
      Blink(LED,5);
    }
  }
  
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    //fill in the struct with new values
    theData.nodeId = NODEID;
    theData.uptime = millis();
    theData.temp = readInternalTemperature(); 
    theData.vcc  = readVcc();
    
    // Display Data we send 
    Serial.print(F("Sending struct["));
    Serial.print(sizeof(theData));
    Serial.print(F("] To nodeId "));
    Serial.print(GATEWAYID);
    Serial.print(F(" with { "));
    Serial.print(F("temp="));
    Serial.print(theData.temp/10.0);
    Serial.print(F(" C  Vcc="));
    Serial.print(theData.vcc);
    Serial.print(F(" mV"));
    Serial.print(F(" uptime="));
    Serial.print(theData.uptime/1000);
    Serial.print(F(" s } ... "));
    
    // request ACK each time, this will permit to get back RSSI from master
    radio.Send(GATEWAYID, (const void*)(&theData), sizeof(theData), true);
    Serial.print(F("waiting for ACK..."));
    if (waitForAck(GATEWAYID)) 
      Serial.print(F("ok!"));
    else 
      Serial.print(F("nothing..."));

    Serial.println();
    Blink(LED,3);
    lastPeriod=currPeriod;
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
