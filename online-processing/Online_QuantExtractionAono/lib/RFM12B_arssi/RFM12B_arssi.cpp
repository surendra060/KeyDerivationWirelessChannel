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

uint8_t RFM12B::cs_pin=SPI_SS;            // CS pin for SPI
uint8_t RFM12B::irq_pin=RFM_DEFAULT_IRQ;  // IRQ Pin default Hardware D2
uint8_t RFM12B::nodeID;                   // address of this node
uint8_t RFM12B::networkID;                // network group ID
long RFM12B::rf12_seq;
uint32_t RFM12B::seqNum;
uint32_t RFM12B::cryptKey[4];
volatile uint8_t RFM12B::rxfill;       // number of data bytes in rf12_buf
#ifdef RF12_ARSSI_ANALOG
  volatile uint16_t RFM12B::arssi_idle=0;// analog rssi idle voltage in mV (0 = default disable)
  volatile uint16_t RFM12B::arssi;       // analog rssi average value
  volatile uint8_t RFM12B::arssi_bytes;  // analog rssi values readed
  volatile uint8_t RFM12B::arssi_config; // Used to indicate no ADC conversion should occurs and analog pin
#endif
volatile int8_t RFM12B::rxstate;       // current transceiver state
volatile uint16_t RFM12B::rf12_crc;    // running crc value
volatile uint8_t rf12_buf[RF_MAX];     // recv/xmit buf, including hdr & crc bytes


/* ======================================================================
Function: SetCS
Purpose : Set the chip select pin used to drive RFM12B
Input   : Arduino Pin (in Arduino Format, ie 7, A3 ...)
Output  : -
Comments: This call should be done BEFORE calling the Initialize
          if you checked device with isPresent with CS and IRQ Pin
          you don't need to call this function again
====================================================================== */
void RFM12B::SetCS(uint8_t arduinoPin)
{
  // Be sure to pull old CS line to High
  digitalWrite(cs_pin, 1);

  // Set new pin
  cs_pin = arduinoPin;

  // Configure pin as output
  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, 1);
}

/* ======================================================================
Function: SetIRQ
Purpose : Set the IRQ pin to use
Input   : 2 or 3
Output  : -
Comments: This call should be done BEFORE calling the Initialize
          if you checked device with isPresent with CS and IRQ Pin
          you don't need to call this function again
====================================================================== */
void RFM12B::SetIRQ(uint8_t irqPin)
{

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)

    //  External IRQ authorized are D2 (INT0) or D3 (INT1)
  if (irq_pin==2 || irq_pin==3)
  {
    // 1st detach the existing one
    detachInterrupt(irq_pin-2);

    // Set new pin
    irq_pin = irqPin;
  }
  else
  {
    irq_pin = RFM_DEFAULT_IRQ;
  }

#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)

  // INT0 or INT1 for pin D10/D11 and INT2 for pin D2
  if (irq_pin==10 || irq_pin==11 || irq_pin==2)
  {
    // 1st detach the existing one
    detachInterrupt(irq_pin==2?2:irq_pin-10);

    irq_pin = irqPin;
  }
  else
  {
    // Set new pin
    irq_pin = RFM_DEFAULT_IRQ;
  }

#else
  #error Target not supported for HW Interrupts
#endif

  // Configure new pin as input with pull up
  pinMode(irq_pin, INPUT);
  digitalWrite(irq_pin, 1);
}

/* ======================================================================
Function: ConfigureInterrupts
Purpose : Configure port and IRQ
Input   : -
Output  : -
Comments: -
====================================================================== */
void RFM12B::ConfigureInterrupts()
{
  // Pin change IRQ
  #if PINCHG_IRQ
    #if RFM_DEFAULT_IRQ < 8
      if (nodeID != 0) {
        bitClear(DDRD, RFM_DEFAULT_IRQ);      // input
        bitSet(PORTD, RFM_DEFAULT_IRQ);       // pull-up
        bitSet(PCMSK2, RFM_DEFAULT_IRQ);      // pin-change
        bitSet(PCICR, PCIE2);         // enable
      } else
        bitClear(PCMSK2, RFM_DEFAULT_IRQ);
    #elif RFM_DEFAULT_IRQ < 14
      if (nodeID != 0) {
        bitClear(DDRB, RFM_DEFAULT_IRQ - 8);  // input
        bitSet(PORTB, RFM_DEFAULT_IRQ - 8);   // pull-up
        bitSet(PCMSK0, RFM_DEFAULT_IRQ - 8);  // pin-change
        bitSet(PCICR, PCIE0);         // enable
      } else
        bitClear(PCMSK0, RFM_DEFAULT_IRQ - 8);
    #else
      if (nodeID != 0) {
        bitClear(DDRC, RFM_DEFAULT_IRQ - 14); // input
        bitSet(PORTC, RFM_DEFAULT_IRQ - 14);  // pull-up
        bitSet(PCMSK1, RFM_DEFAULT_IRQ - 14); // pin-change
        bitSet(PCICR, PCIE1);         // enable
      } else
        bitClear(PCMSK1, RFM_DEFAULT_IRQ - 14);
    #endif
  #else
    #if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
      //  External IRQ authorized are D2 (INT0) or D3 (INT1)
      if (nodeID != 0)
        // INT0 or INT1
        attachInterrupt(irq_pin-2, RFM12B::InterruptHandler, LOW);
      else
        // INT0 or INT1
        detachInterrupt(irq_pin-2);
    #elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
      //  IRQ authorized are D10 (INT0), D11 (INT1) or D2 (INT2)
      if (irq_pin==10 || irq_pin==11 || irq_pin==2)
      {
        if (nodeID != 0)
          // INT0 or INT1 for pin D10/D11 and INT2 for pin D2
          attachInterrupt(irq_pin==2?2:irq_pin-10, RFM12B::InterruptHandler, LOW);
        else
          // INT0 or INT1 for pin D10/D11 and INT2 for pin D2
          detachInterrupt(irq_pin==2?2:irq_pin-10);
      }
    #else
      #error Target not supported for HW Interrupts
    #endif
  #endif
}

/* ======================================================================
Function: isPresent
Purpose : try to detect RFM12B device connected to SPI bus
Input   : CS Pin (if not default D10)
          IRQ Pin (if not default D2)
Output  : True if found false otherwise
Comments: duration of 20 ms of wait state.
====================================================================== */
bool RFM12B::isPresent(uint8_t cspin, uint8_t irqpin)
{
  bool found = false;
  unsigned long start_to;

    // Set hardware we use
  SetCS(cspin);
  SetIRQ(irqpin);

  // Init RFM12B module
  // whatever values here, the real init should be done later
  // We just need to init the device
  if ( Initialize(1, RF12_433MHZ) )
  {

    // Clock output (1.66MHz), Low Voltage threshold (3.75V)
    // we will change the Low Voltage threshold to the highest
    // value we can, as the module is powered by 3.3V the bit
    // indicating low voltage should now be set because at 3.3V
    // we are under 3.75V
    Control(0xC04F);

    // Wait the voltage comparison to settle ?
    // I do not have any idea if needed, but it can't hurt
    delay(10);

    // Check if module is low battery ?
    if ( LowBattery() )
    {
      // Here we can have false detection in case of NRF24 plugged
      // since we just check SPI return on one bit (LowVoltage)
      // Clock output (1.66MHz), Low Voltage threshold (2.25V)
      // So set voltage detection to test inverted bit
      Control(0xC040);

      delay(10);

      // Check if module is no more low battery ?
      // this time sounds we got a RFM12B module
      if ( !LowBattery() )
        found = true;
    }

    // Adjust low battery voltage to POR value
    Control(0xC000);
  }

  return found;
}


void RFM12B::SPIInit()
{
  // avoid Arduino to be SPI slave setting SS Pin as output
  if (cs_pin != SS )
  {
    pinMode(SS, OUTPUT);
  //digitalWrite(10, 1);
  }

  pinMode(cs_pin, OUTPUT);
  digitalWrite(cs_pin, 1);
  pinMode(SPI_MOSI, OUTPUT);
  pinMode(SPI_MISO, INPUT);
  pinMode(SPI_SCK, OUTPUT);
#ifdef SPCR
  SPCR = _BV(SPE) | _BV(MSTR);
#if F_CPU > 10000000
  // use clk/2 (2x 1/4th) for sending (and clk/8 for recv, see XFERSlow)
  SPSR |= _BV(SPI2X);
#endif
#else
  // ATtiny
  USICR = bit(USIWM0);
#endif
  pinMode(irq_pin, INPUT);
  digitalWrite(irq_pin, 1); // pull-up
}

uint8_t RFM12B::Byte(uint8_t out) {
  SPDR = out;
  // this loop spins 4 usec with a 2 MHz SPI clock
  while (!(SPSR & _BV(SPIF)));
  return SPDR;
}

uint16_t RFM12B::XFERSlow(uint16_t cmd) {
  // slow down to under 2.5 MHz
#if F_CPU > 10000000
  bitSet(SPCR, SPR0);
#endif
  //DisableInterrupts();
  digitalWrite(cs_pin, 0);
  uint16_t reply = Byte(cmd >> 8) << 8;
  reply |= Byte(cmd);
  digitalWrite(cs_pin, 1);
  //EnableInterrupts();
#if F_CPU > 10000000
  bitClear(SPCR, SPR0);
#endif
  return reply;
}

void RFM12B::XFER(uint16_t cmd) {
#if OPTIMIZE_SPI
  // writing can take place at full speed, even 8 MHz works
  //DisableInterrupts();
  digitalWrite(cs_pin, 0);

  Byte(cmd >> 8) << 8;
  Byte(cmd);
  digitalWrite(cs_pin, 1);
  //EnableInterrupts();
#else
  XFERSlow(cmd);
#endif
}



// Call this once with params:
// - node ID (0-31)
// - frequency band (RF12_433MHZ, RF12_868MHZ, RF12_915MHZ)
// - networkid [optional - default = 170] (0-255 for RF12B, only 212 allowed for RF12)
// - txPower [optional - default = 0 (max)] (7 is min value)
// - AirKbps [optional - default = 38.31Kbps]
// - lowVoltageThreshold [optional - default = RF12_2v75]
bool RFM12B::Initialize(uint8_t ID, uint8_t freqBand, uint8_t networkid, uint8_t txPower, uint8_t airKbps, uint8_t lowVoltageThreshold)
{
  unsigned long start_to;

  // since we can call this several times, verify that we clear all interrupts before restarting
  // Calling ConfigureInterrupts with NodeID=0 disable them
  nodeID = 0;

  ConfigureInterrupts();
  nodeID = ID;
  networkID = networkid;
  SPIInit();
  XFER(0x0000); // intitial SPI transfer added to avoid power-up problem
  XFER(RF_SLEEP_MODE); // DC (disable clk pin), enable lbd

  // wait until RFM12B is out of power-up reset, this takes several *seconds*
  XFER(RF_TXREG_WRITE); // in case we're still in OOK mode

  #define TIME_OUT 200
  start_to = millis();

  while (digitalRead(irq_pin)==0 && millis()-start_to < TIME_OUT)
    XFER(0x0000);

  if (millis()-start_to >= TIME_OUT)
    return (false);

  XFER(0x80C7 | (freqBand << 4)); // EL (ena TX), EF (ena RX FIFO), 12.0pF
  XFER(0xA640); // Frequency is exactly 434/868/915MHz (whatever freqBand is)
  XFER(0xC600 + airKbps);   //Air transmission baud rate: 0x08= ~38.31Kbps
  XFER(0x94A2);             // VDI,FAST,134kHz,0dBm,-91dBm
  XFER(0xC2AC);             // AL,!ml,DIG,DQD4
  if (networkID != 0) {
    XFER(0xCA83);           // FIFO8,2-SYNC,!ff,DR
    XFER(0xCE00 | networkID); // SYNC=2DXX;
  } else {
    XFER(0xCA8B); // FIFO8,1-SYNC,!ff,DR
    XFER(0xCE2D); // SYNC=2D;
  }
  XFER(0xC483); // @PWR,NO RSTRIC,!st,!fi,OE,EN
  XFER(0x9850 | (txPower > 7 ? 7 : txPower)); // !mp,90kHz,MAX OUT               //last byte=power level: 0=highest, 7=lowest
  XFER(0xCC77); // OB1,OB0, LPX,!ddy,DDIT,BW0
  XFER(0xE000); // NOT USE
  XFER(0xC800); // NOT USE
  XFER(0xC043); // Clock output (1.66MHz), Low Voltage threshold (2.55V)

  rxstate = TXIDLE;

  // Configure IRQ Settings
  ConfigureInterrupts();

  return (true);
}

// access to the RFM12B internal registers with interrupts disabled
uint16_t RFM12B::Control(uint16_t cmd) {
#ifdef EIMSK
    cli();
    uint16_t r = XFERSlow(cmd);
    sei();
#else
  // ATtiny
  cli();
  uint16_t r = XFERSlow(cmd);
  sei();
#endif
    return r;
}

#ifdef RF12_ARSSI_ANALOG
  /* ======================================================================
  Function: noRSSI
  Purpose : Indicate the driver that main app is using ADC
  Input   : true if caller is using ADC (so disable RSSI for RFM12B driver)
            false to release ADC reading in RFM12B driver
  Output  : -
  Comments: This call should be done BEFORE calling the Initialize
  ====================================================================== */
  void RFM12B::noRSSI(bool _state)
  {
    cli();
    if (_state)
      arssi_config |= RF12_ARSSI_NO_ADC_MASK;
    else
      arssi_config &= ~RF12_ARSSI_NO_ADC_MASK;
    sei();
  }

  /* ======================================================================
  Function: SetRSSI
  Purpose : Initialise ARSSI settings
  Input   : Analog pin to use in ADMUX pin format so 0..7 = A0..A7
            Value of ARSSI Idle in mV (typically 300 to 700)
  Output  : -
  Comments: When setting ARSSI idle to 0, this disable ARSSI reading
  ====================================================================== */
  void RFM12B::SetRSSI(uint8_t _analog_pin, uint16_t _arssi_idle)
  {
    arssi_idle = _arssi_idle;
    pinMode(_analog_pin+A0, INPUT);
    cli();
    arssi_config &= ~RF12_ARSSI_ANALOG_PIN_MASK;
    arssi_config |= (_analog_pin & RF12_ARSSI_ANALOG_PIN_MASK);
    sei();
  }

/* ======================================================================
  Function: ReadARSSI
  Purpose : Read ARSSI calculated by last frame received by RFM12B
  Input   : value in mV of VCC (ie 3300 for 3.3V powered Arduino
  Output  : value of ARSSI (in dB) or const defined below
  Comments: Read ARSSI connected to Analog pin
            http://www.hoperf.com/upload/rf/RFm12b.pdf
            says 450mV to 1150mV this is what I measured on RFM12B chip
            on some other module (example with visible chipset)
            http://www.silabs.com/Support%20Documents/TechnicalDocs/Si4420.pdf
            says 300mV to 1000mV or 600mV to 1300mV on mine I measured 600mV
            That said, to calculated correct ARSSI we need to know the VCC range (used by ADC)
            And also the idle offset depending on chipset (300mV, 450mV or 600mV)
            this is the role on _arssi_idle parameter of SetRSSI function above
            The return value is in range -100db to -65db
            special values are :

            RF12_ARSSI_ABOVE_MAX  (-64)
                calculated values above -65db should not happen

            RF12_ARSSI_BELOW_MIN  (-101)
                calculated values below -100db should not happen

            RF12_ARSSI_RECV       (-102)
                RFM12B module is currently receiving

            RF12_ARSSI_BAD_IDLE   (-103)
                _arssi_idle parameter of SetRSSI incorrect
                should be one the the defined rfm12b.h file

            RF12_ARSSI_DISABLED (-104)
                ARSSI disable

            RF12_ARSSI_NB_BYTES (-110 to -116 )
                not enough bytes received (need 6 minimum)
                -110= 0 bytes, -111=1 byte, -112=2 bytes, ...
  ====================================================================== */
  int8_t RFM12B::ReadARSSI(uint16_t _vcc)
  {
    uint16_t _arssi;
    uint16_t vrssi;
    int8_t  rssi;
    int8_t  _arssi_bytes;
    int16_t  _vmax;

    // Still not finished receiving ?
    // special value indicating we're receiving
    // so value can be modified in IRQ so not reliable
    if (rxstate != TXIDLE)
      return (RF12_ARSSI_RECV);

    // We did not activated the arssi function at init
    if (arssi_idle==0)
      return RF12_ARSSI_DISABLED;

    // be sure all is fine while getting eventually modified
    // values by IRQ, so disable IRQ
    cli();

    // Get the arssi needed values
    _arssi = arssi;
    _arssi_bytes = arssi_bytes;

    // we done, all is now fine
    sei();

    // indicate values not reliable + nb byte read so
    // -102 to -108
    if (_arssi_bytes < RF12_ARSSI_MIN_BYTES)
      return (RF12_ARSSI_NB_BYTES-_arssi_bytes);

    //Serial.print(_arssi);
    //Serial.print("/");
    //Serial.print(_arssi_bytes);
    //Serial.print("=");

    // Calculate the arssi average value
    _arssi = _arssi/_arssi_bytes;

    //Serial.print(_arssi);
    //Serial.print(" ");

    // convert arssi value to mV (cast to bigger size for calculation)
    vrssi = ((uint32_t) _vcc * _arssi / 1024);

    //Serial.print(F("vrssi measured "));
    //Serial.print(vrssi);
    //Serial.print(F("mV "));

    // Checkup what is ARSSI idle voltage
    // max is always idle + 700 mV
    // Indicate and remove offset
    if ( arssi_idle>=RF12_ARSSI_300mV && arssi_idle<=RF12_ARSSI_600mV )
      _vmax = arssi_idle + RF12_ARSSI_maxmV ;
    else
      return (RF12_ARSSI_BAD_IDLE);

    //Serial.print(F("arssi_idle "));
    //Serial.print(arssi_idle);
    //Serial.print(F("mV "));

    // Remove idle offset voltage
    // vmax -= arssi_idle ;
    //vrssi -= arssi_idle ;

    //Serial.print(F("vrssi "));
    //Serial.print(vrssi);
    //Serial.print(F("mV "));

    //Serial.print(F("vmax "));
    //Serial.print(_vmax);
    //Serial.print(F("mV "));

    // calculate arssi value (from 0 to 35)
    // (-100 to -65 is 35), 700 is delta between max and min
    rssi = (( vrssi - arssi_idle ) / (700/35));

    // Add -100 db offset
    rssi -= 100;

    // check limits
    if (rssi <= RF12_ARSSI_MIN) return (RF12_ARSSI_BELOW_MIN);
    if (rssi >= RF12_ARSSI_MAX) return (RF12_ARSSI_ABOVE_MAX);

    return rssi;
  }
#endif

void RFM12B::InterruptHandler() {
  // a transfer of 2x 16 bits @ 2 MHz over SPI takes 2x 8 us inside this ISR
  // correction: now takes 2 + 8 µs, since sending can be done at 8 MHz
  XFER(0x0000);


  if (rxstate == TXRECV)
  {
    uint8_t in ;
    uint16_t adc_value;

    #ifdef RF12_ARSSI_ANALOG
      // function activated ?
      if (arssi_idle)
      {
        // we can take the ADC ? if so configure it
        if (!(arssi_config & RF12_ARSSI_NO_ADC_MASK))
        {
          #if F_CPU >= 16000000L
            // At 16MHz set prescaler to 32 (500Khz => 32 us per sample)
            ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS0);
          #else
            // At 8MHz set prescaler to 16 (500Khz => 32 us per sample)
            ADCSRA = _BV(ADEN) | _BV(ADPS2) ;
          #endif

          // ADMUX ADC configuration for RSSI reading, set channel
          ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (arssi_config & RF12_ARSSI_ANALOG_PIN_MASK) ;

          // Start 1st Conversion, but we will ignore it
          ADCSRA |= _BV(ADSC);
        }
      }
    #endif

    in = XFERSlow(RF_RX_FIFO_READ);

    #ifdef RF12_ARSSI_ANALOG
      // function activated ?
      if (arssi_idle)
      {
        // we can take the ADC ?
        if (!(arssi_config & RF12_ARSSI_NO_ADC_MASK))
        {
          // Wait end of the 1st conversion and start 2nd
          while (bit_is_set(ADCSRA,ADSC));
          ADCSRA |= _BV(ADSC);
        }
      }
    #endif

    if (rxfill == 0 )
    {
      if (networkID != 0)
        rf12_buf[rxfill++] = networkID;
    }

    //Serial.print(in, HEX); Serial.print(' ');
    rf12_buf[rxfill++] = in;
    rf12_crc = _crc16_update(rf12_crc, in);

    #ifdef RF12_ARSSI_ANALOG
      // function activated ?
      if (arssi_idle)
      {
        // we can take the ADC and we can rely on value?
        if ((arssi_config & RF12_ARSSI_NO_ADC_MASK)==0 && arssi_bytes <=128 )
        {
          // Wait till conversion done
          while (bit_is_set(ADCSRA,ADSC));

          // read low first
          adc_value = ADCL;
          adc_value |= (ADCH << 8 );

          // Datasheet says RSSI pin is 300mV min and 1300mV max, so with 3V3 power ADC value should be
          // ADC range / (3.3V)  / 1024
          // (3300 / 1024) => 3.22 so 3.22mV by adc reading
          // Says we consider data valid from 300mV to 1300mV
          // give us 93 = 300mV and 404 give us 1300mV, let's set up 90 and 410
          // Take care that if powering Atmel at 5V you need to calculate back theese limits
          // In 5V values are 60 and 270 instead of 90 and 410

          // I setup the min for 5V and max for 3V3, should work in both case
          // Ignore value should above max and eventually overflowing our arssi counter
          if (adc_value >= 60 && adc_value <= 410)
          {
            arssi += adc_value;
            arssi_bytes++;
          }
        }
      }
    #endif

    if (rxfill >= rf12_len + 6 || rxfill >= RF_MAX)
      XFER(RF_IDLE_MODE);
  } else {
    uint8_t out;

      if (rxstate < 0) {
        uint8_t pos = 4 + rf12_len + rxstate++;
        out = rf12_buf[pos];
        rf12_crc = _crc16_update(rf12_crc, out);
      } else
        switch (rxstate++) {
          case TXSYN1: out = 0x2D; break;
          case TXSYN2: out = networkID; rxstate = -(3 + rf12_len); break;
          case TXCRC1: out = rf12_crc; break;
          case TXCRC2: out = rf12_crc >> 8; break;
          case TXDONE: XFER(RF_IDLE_MODE); // fall through
          default:     out = 0xAA;
        }

    //Serial.print(out, HEX); Serial.print(' ');
    XFER(RF_TXREG_WRITE + out);
  }
}


#if PINCHG_IRQ
  #if RFM_IRQ < 8
    ISR(PCINT2_vect) {
      while (!bitRead(PIND, RFM_DEFAULT_IRQ))
        RFM12B::InterruptHandler();
    }
  #elif RFM_IRQ < 14
    ISR(PCINT0_vect) {
      while (!bitRead(PINB, RFM_DEFAULT_IRQ - 8))
        RFM12B::InterruptHandler();
    }
  #else
    ISR(PCINT1_vect) {
      while (!bitRead(PINC, RFM_DEFAULT_IRQ - 14))
        RFM12B::InterruptHandler();
    }
  #endif
#endif

void RFM12B::ReceiveStart() {
  rxfill = rf12_len = 0;
  rf12_crc = ~0;
  if (networkID != 0)
    rf12_crc = _crc16_update(~0, networkID);

  #ifdef RF12_ARSSI_ANALOG
    // Initialize RSSI readings

    arssi = 0;
    arssi_bytes=0;

  #endif

  rxstate = TXRECV;
  XFER(RF_RECEIVER_ON);
}

bool RFM12B::ReceiveComplete() {
  if (rxstate == TXRECV && (rxfill >= rf12_len + 6 || rxfill >= RF_MAX)) {
    rxstate = TXIDLE;
    if (rf12_len > RF12_MAXDATA)
      rf12_crc = 1; // force bad crc if packet length is invalid
    if (RF12_DESTID == 0 || RF12_DESTID == nodeID) { //if (!(rf12_hdr & RF12_HDR_DST) || (nodeID & NODE_ID) == 31 || (rf12_hdr & RF12_HDR_MASK) == (nodeID & NODE_ID)) {
      if (rf12_crc == 0 && crypter != 0)
        crypter(false);
      else
        rf12_seq = -1;
      return true; // it's a broadcast packet or it's addressed to this node
    }
  }
  if (rxstate == TXIDLE){
    ReceiveStart();
  }
  return false;
}


bool RFM12B::CanSend() {
  // no need to test with interrupts disabled: state TXRECV is only reached
  // outside of ISR and we don't care if rxfill jumps from 0 to 1 here
  if (rxstate == TXRECV && rxfill == 0 && (Byte(0x00) & (RF_RSSI_BIT >> 8)) == 0) {
    XFER(RF_IDLE_MODE); // stop receiver
    //XXX just in case, don't know whether these RF12 reads are needed!
    // rf12_XFER(0x0000); // status register
    // rf12_XFER(RF_RX_FIFO_READ); // fifo read
    rxstate = TXIDLE;
    return true;
  }
  return false;
}

void RFM12B::SendStart(uint8_t toNodeID, bool requestACK, bool sendACK) {
  rf12_hdr1 = toNodeID | (sendACK ? RF12_HDR_ACKCTLMASK : 0);
  rf12_hdr2 = nodeID | (requestACK ? RF12_HDR_ACKCTLMASK : 0);
  if (crypter != 0) crypter(true);
  rf12_crc = ~0;
  rf12_crc = _crc16_update(rf12_crc, networkID);
  rxstate = TXPRE1;
  XFER(RF_XMITTER_ON); // bytes will be fed via interrupts
}

void RFM12B::SendStart(uint8_t toNodeID, const void* sendBuf, uint8_t sendLen, bool requestACK, bool sendACK, uint8_t waitMode) {
  rf12_len = sendLen;
  memcpy((void*) rf12_data, sendBuf, sendLen);
  SendStart(toNodeID, requestACK, sendACK);
  SendWait(waitMode);
}

/// Should be called immediately after reception in case sender wants ACK
void RFM12B::SendACK(const void* sendBuf, uint8_t sendLen, uint8_t waitMode) {
  while (!CanSend()) ReceiveComplete();
  SendStart(RF12_SOURCEID, sendBuf, sendLen, false, true, waitMode);
}

void RFM12B::Send(uint8_t toNodeID, const void* sendBuf, uint8_t sendLen, bool requestACK, uint8_t waitMode)
{
  while (!CanSend()) ReceiveComplete();
  SendStart(toNodeID, sendBuf, sendLen, requestACK, false, waitMode);
}

void RFM12B::SendWait(uint8_t waitMode) {
  // wait for packet to actually finish sending
  // go into low power mode, as interrupts are going to come in very soon
  while (rxstate != TXIDLE)
    if (waitMode) {
      // power down mode is only possible if the fuses are set to start
      // up in 258 clock cycles, i.e. approx 4 us - else must use standby!
      // modes 2 and higher may lose a few clock timer ticks
      set_sleep_mode(waitMode == 3 ? SLEEP_MODE_PWR_DOWN :
      #ifdef SLEEP_MODE_STANDBY
                     waitMode == 2 ? SLEEP_MODE_STANDBY :
      #endif
                     SLEEP_MODE_IDLE);
      sleep_mode();
    }
}

void RFM12B::OnOff(uint8_t value) {
  XFER(value ? RF_XMITTER_ON : RF_IDLE_MODE);
}

void RFM12B::Sleep(char n) {
  if (n < 0)
    Control(RF_IDLE_MODE);
  else {
    Control(RF_WAKEUP_TIMER | 0x0500 | n);
    Control(RF_SLEEP_MODE);
    if (n > 0)
      Control(RF_WAKEUP_MODE);
  }
  rxstate = TXIDLE;
}
void RFM12B::Sleep() { Sleep(0); }
void RFM12B::Wakeup() { Sleep(-1); }

bool RFM12B::LowBattery() {
  return (Control(0x0000) & RF_LBD_BIT) != 0;
}

uint8_t RFM12B::GetSender(){
  return RF12_SOURCEID;
}


volatile uint8_t * RFM12B::GetData() { return rf12_data; }
uint8_t RFM12B::GetDataLen() { return *DataLen; }
bool RFM12B::ACKRequested() { return RF12_WANTS_ACK; }

/// Should be polled immediately after sending a packet with ACK request
bool RFM12B::ACKReceived(uint8_t fromNodeID) {
  if (ReceiveComplete())
    return CRCPass() &&
           RF12_DESTID == nodeID &&
          (RF12_SOURCEID == fromNodeID || fromNodeID == 0) &&
          (rf12_hdr1 & RF12_HDR_ACKCTLMASK) &&
          !(rf12_hdr2 & RF12_HDR_ACKCTLMASK);
  return false;
}
// XXTEA by David Wheeler, adapted from http://en.wikipedia.org/wiki/XXTEA
#define DELTA 0x9E3779B9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (cryptKey[(uint8_t)((p&3)^e)] ^ z)))
void RFM12B::CryptFunction(bool sending) {
  uint32_t y, z, sum, *v = (uint32_t*) rf12_data;
  uint8_t p, e, rounds = 6;

  if (sending) {
    // pad with 1..4-byte sequence number
    *(uint32_t*)(rf12_data + rf12_len) = ++seqNum;
    uint8_t pad = 3 - (rf12_len & 3);
    rf12_len += pad;
    rf12_data[rf12_len] &= 0x3F;
    rf12_data[rf12_len] |= pad << 6;
    ++rf12_len;
    // actual encoding
    char n = rf12_len / 4;
    if (n > 1) {
      sum = 0;
      z = v[n-1];
      do {
        sum += DELTA;
        e = (sum >> 2) & 3;
        for (p=0; p<n-1; p++)
            y = v[p+1], z = v[p] += MX;
        y = v[0];
        z = v[n-1] += MX;
      } while (--rounds);
    }
  } else if (rf12_crc == 0) {
    // actual decoding
    char n = rf12_len / 4;
    if (n > 1) {
      sum = rounds*DELTA;
      y = v[0];
      do {
        e = (sum >> 2) & 3;
        for (p=n-1; p>0; p--)
          z = v[p-1], y = v[p] -= MX;
        z = v[n-1];
        y = v[0] -= MX;
      } while ((sum -= DELTA) != 0);
    }
    // strip sequence number from the end again
    if (n > 0) {
      uint8_t pad = rf12_data[--rf12_len] >> 6;
      rf12_seq = rf12_data[rf12_len] & 0x3F;
      while (pad-- > 0)
        rf12_seq = (rf12_seq << 8) | rf12_data[--rf12_len];
    }
  }
}

void RFM12B::Encrypt(const uint8_t* key, uint8_t keyLen) {
  // by using a pointer to CryptFunction, we only link it in when actually used
  if (key != 0) {
    for (uint8_t i = 0; i < keyLen; ++i)
      ((uint8_t*) cryptKey)[i] = key[i];
    crypter = CryptFunction;
  } else crypter = 0;
}
