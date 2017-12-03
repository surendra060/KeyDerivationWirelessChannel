#include <Wire.h>
#include <MicroLCD.h>
#include <RFM12B_arssi.h>

#define NODEID      99
#define NETWORKID   100
#define GATEWAYID   1
#define RFM_CS_PIN  10 // RFM12B Chip Select Pin
#define RFM_IRQ_PIN 2  // RFM12B IRQ Pin
#define FREQUENCY   RF12_433MHZ //Match this with the version of your Moteino! (others: RF69_433MHZ, RF69_868MHZ)
#define SERIAL_BAUD 115200
#define ACK_TIME    30  // # of ms to wait for an ack


#define BOARD_ARDUINODE_V_1_1

#ifdef BOARD_ARDUINODE_V_1_1
	#define LED_RED					3
	#define LED_BLU					4
	#define LED_GRN					5
	#define SW_IRQ					6
	#define LED_RF					8
	#define RF_POWER_PIN		9
	#define OLED_RESET			A2
  #define LED        LED_GRN
#else
  #define LED             9
#endif

int TRANSMITPERIOD = 1000; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
RFM12B radio;
long lastPeriod = -1;

typedef struct {		
  uint8_t   command;// command identifier 
  uint16_t  vcc;    // current Arduino VCC power 
  int8_t    rssi;   // RSSI
} Payload;
Payload theData;

LCD_SSD1306 lcd; /* for SSD1306 OLED module */

const PROGMEM uint8_t smile[48 * 48 / 8] = {
0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xF8,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xF8,0xF8,0xF0,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0xC0,0xF0,0xFC,0xFE,0xFF,0xFF,0xFF,0x3F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80,0x80,0x80,0x00,0x01,0x03,0x07,0x0F,0x1F,0x3F,0xFF,0xFF,0xFF,0xFE,0xFC,0xF0,0xC0,0x00,
0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x06,0x1F,0x1F,0x1F,0x3F,0x1F,0x1F,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,
0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x00,0x30,0xF8,0xF8,0xF8,0xF8,0xE0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xE0,0xF8,0xF8,0xFC,0xF8,0x30,0x00,0x00,0xE0,0xFF,0xFF,0xFF,0xFF,0xFF,0x7F,
0x00,0x03,0x0F,0x3F,0x7F,0xFF,0xFF,0xFF,0xFC,0xF8,0xF0,0xE1,0xC7,0x87,0x0F,0x1F,0x3F,0x3F,0x3E,0x7E,0x7C,0x7C,0x7C,0x78,0x78,0x7C,0x7C,0x7C,0x7E,0x3E,0x3F,0x3F,0x1F,0x0F,0x87,0xC7,0xE1,0xF0,0xF8,0xFC,0xFF,0xFF,0xFF,0x7F,0x3F,0x0F,0x03,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F,0x3F,0x3F,0x7F,0x7F,0x7E,0xFE,0xFE,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFC,0xFE,0xFE,0x7E,0x7F,0x7F,0x3F,0x3F,0x1F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
};

const PROGMEM uint8_t tick[16 * 16 / 8] =
{0x00,0x80,0xC0,0xE0,0xC0,0x80,0x00,0x80,0xC0,0xE0,0xF0,0xF8,0xFC,0x78,0x30,0x00,0x00,0x01,0x03,0x07,0x0F,0x1F,0x1F,0x1F,0x0F,0x07,0x03,0x01,0x00,0x00,0x00,0x00};

const PROGMEM uint8_t cross[16 * 16 / 8] =
{0x00,0x0C,0x1C,0x3C,0x78,0xF0,0xE0,0xC0,0xE0,0xF0,0x78,0x3C,0x1C,0x0C,0x00,0x00,0x00,0x30,0x38,0x3C,0x1E,0x0F,0x07,0x03,0x07,0x0F,0x1E,0x3C,0x38,0x30,0x00,0x00};

const PROGMEM uint8_t bar_full[16 * 16 / 8] =
{0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const PROGMEM uint8_t bar_empty[16 * 16 / 8] =
{0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const PROGMEM uint8_t bar_full8[8 * 8 / 8] =
{0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const PROGMEM uint8_t bar_full1[8 * 1 / 8] =
{0xFF};

const PROGMEM uint8_t bar_start[8 * 16 / 8] = 
{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF};

const PROGMEM uint8_t bar_end[8 * 16 / 8] =   
{0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00, 0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

const PROGMEM uint8_t bar[][8 * 16 / 8] = 
{
  {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
  {0xFF,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0xFF,0x80,0x80,0x80,0x80,0x80,0x80,0x80},
  {0xFF,0xFF,0x01,0x01,0x01,0x01,0x01,0x01,0xFF,0xFF,0x80,0x80,0x80,0x80,0x80,0x80},
  {0xFF,0xFF,0xFF,0x01,0x01,0x01,0x01,0x01,0xFF,0xFF,0xFF,0x80,0x80,0x80,0x80,0x80},
  {0xFF,0xFF,0xFF,0xFF,0x01,0x01,0x01,0x01,0xFF,0xFF,0xFF,0xFF,0x80,0x80,0x80,0x80},
  {0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x80,0x80},
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x80},
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80},
  {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}
};




uint16_t vcc=0;
volatile uint8_t  adc_irq_cnt;

/* ======================================================================
Function: drawBargraph
Purpose : 
Input   : 
Output  : 
Comments: 
====================================================================== */
void drawBargraph(uint8_t x, uint8_t y, uint8_t value, uint8_t max_value, uint8_t nb_bar)
{
  uint8_t index;
  uint8_t nb_char = nb_bar / 8;

  lcd.setCursor(x, y);
  lcd.draw(bar_start, 8, 16);
  
  nb_bar = map(value, 0, max_value, 0, nb_bar);

  for (uint8_t i=1; i<=nb_char; i++)
  {
    lcd.setCursor(x+(i*8), y);
    index = (nb_bar>8) ? 8 : nb_bar;
    lcd.draw(bar[index], 8, 16);
    nb_bar = (nb_bar>8) ? nb_bar-=8 : 0;
  }

  lcd.setCursor(x+(nb_char+1)*8, y);
  lcd.draw(bar_end, 8, 16);
}

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
Function: setup
Purpose : Configuration of Arduino I/O and other stuff
Input   : -
Output  : -
Comments: 
====================================================================== */
void setup() 
{

  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);
  
  #ifdef BOARD_ARDUINODE_V_1_1
    pinMode(LED_RF, OUTPUT);
    digitalWrite(LED_RF, HIGH);
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_RED, LOW);
//    pinMode(LED_BLU, OUTPUT);
    pinMode(LED_GRN, OUTPUT);
  digitalWrite(LED_GRN, LOW);
    // Enable RF Power
    pinMode(RF_POWER_PIN, OUTPUT);
    digitalWrite(RF_POWER_PIN, 0);
    // Enable SWitch Input
    pinMode(SW_IRQ, INPUT);
    digitalWrite(SW_IRQ, 1);
  #endif

  Serial.begin(SERIAL_BAUD);
  Serial.println(F("RFM12B Range Test Node"));
  
 	lcd.begin();
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_MEDIUM);

/*  
  lcd.setCursor(32, 2);
  for (uint8_t i=0; i<=35; i++ )
  {
    // display bargraph on lcd
    drawBargraph(16, 2, i, 35, 64);
    delay(100);
  }
*/
  
  lcd.clear();
  lcd.setFontSize(FONT_SIZE_MEDIUM);
  lcd.setCursor(0, 0);
  lcd.println(F("RF Range Test"));
  lcd.print(F("RFM12B  "));
 	//lcd.setCursor(72, 2);

  // Try to detect and Init On moteino RF12 device SS is D10 and IRQ D2 (default)
  // So the parameters are optional
  // if radio.isPresent()
  if ( radio.isPresent( RFM_CS_PIN, RFM_IRQ_PIN) )
  {
    Serial.println(F("RFM12B Detected OK!"));
    lcd.draw(tick, 16, 16);
  }
  else
  {
    Serial.println(F("RFM12B Detection FAIL! (is chip present?)"));
    lcd.draw(cross, 16, 16);
  }

  
  radio.Initialize(NODEID, FREQUENCY, NETWORKID, 0);
  
  Serial.print(F("Transmitting at "));
  Serial.print(FREQUENCY==RF12_433MHZ ? 433 : FREQUENCY==RF12_868MHZ ? 868 : 915);
  Serial.println(F("Mhz..."));

  lcd.setCursor(0, 4);
  lcd.print(F("ARSSI : "));
  Serial.print(F("ARSSI "));

  
  // If RSSI is activated for this board
  // display relative information
  if ( radio.getRSSIIdle())
  {
    Serial.print(F("Enabled\nConnect ARSSI signal on Analog pin "));
    Serial.print(radio.getRSSIAnalogPin());
    Serial.print(F(" of this board\nARSSI idle voltage is set to "));
    Serial.print(radio.getRSSIIdle());
    Serial.println(F(" mV"));
    lcd.draw(tick, 16, 16);
    lcd.setCursor(0,6);
    lcd.print(F("ARSSI "));
    lcd.print(radio.getRSSIIdle());
    lcd.print(F(" mV on A"));
    lcd.println(radio.getRSSIAnalogPin());
  }
  else
  {
    Serial.println(F("Disabled for this board"));
    lcd.draw(cross, 16, 16);
  }

  // read vcc value
  vcc=readVcc();
  Serial.print(F("Vcc = "));
  Serial.print(vcc);
  Serial.println(F(" mV"));

  
  Serial.println(F("Quick command summary, type it any time"));
  Serial.println(F("  v : read and display VCC"));

  Serial.println(F("\nStarting to send data to gateway ...\n"));
  
  delay(2500);
  lcd.clear();
}

/* ======================================================================
Function: loop
Purpose : really need explanations ???
Input   : -
Output  : -
Comments: 
====================================================================== */
void loop() 
{
  uint8_t  col;
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
    // Read and display VCC value
    if (input == 'v') 
    {
      Serial.print(F("VCC : "));
      Serial.print(readVcc());
      Serial.println(F("mV"));
    }
  }

  //check for any received packets
  if (radio.ReceiveComplete())
  {
    if (radio.CRCPass())
    {
      int8_t rssi, k;
      
      // Light on the led
      digitalWrite(LED,HIGH);
      
      Serial.print('[');Serial.print(radio.GetSender(), DEC);Serial.print("] ");
      
      // len should be 1 (RSSI byte)
      if (*radio.DataLen != 1)
        Serial.print(F("Invalid RSSI payload received, not matching RSSI ACK Format!"));
      else
        rssi = radio.Data[0]; //radio.DATA actually contains RSSI

      
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
        // display bargraph on serial
        Serial.print(F("["));
        
        for (k=RF12_ARSSI_MIN; k<=RF12_ARSSI_MAX; k++)
        {
          if (rssi>=k )
            Serial.print('=');
          else
            Serial.print(' ');
        }

        Serial.print(F("] "));
        Serial.print(rssi);
        Serial.print(F(" dB"));
        
        
        // display bargraph on lcd
        drawBargraph(0, 2, 100+rssi, 35, 64);
        
        lcd.setCursor(32, 5);
        lcd.setFontSize(FONT_SIZE_XLARGE);
        lcd.print(F("-"));
        lcd.printInt(-rssi);
        lcd.print(F(" dB"));
      }


      Serial.println();
  
      // Light off the led
      digitalWrite(LED,LOW);

    }
  }
  
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    lcd.setCursor(0, 0);
    lcd.setFontSize(FONT_SIZE_MEDIUM);
    lcd.print("Sending to #");
    lcd.print(GATEWAYID);
    
    //fill in the payload with values
    theData.command = 0x01;
    theData.vcc  = readVcc();
    theData.rssi = RF12_ARSSI_RECV;
    
    // Light on the led
    digitalWrite(LED_RED,HIGH);

    // No ACK, but payload command permit to get back RSSI from master
    radio.Send(GATEWAYID, (const void*)(&theData), sizeof(theData), false);

    // Light off the led
    digitalWrite(LED_RED,LOW);

    Serial.println();
    lastPeriod=currPeriod;
  }
}

