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

#ifndef RFM12B_ARSSI_H
#define RFM12B_ARSSI_H

#include <inttypes.h>
#include <avr/io.h>
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/sleep.h>
#include <Arduino.h> 

///RF12 Driver version
// #define OPTIMIZE_SPI       1  // uncomment this to write to the RFM12B @ 8 Mhz

/// RF12 CTL bit mask.
//#define RF12_HDR_CTL    0x80
/// RF12 DST bit mask.
//#define RF12_HDR_DST    0x40
/// RF12 ACK bit mask.
//#define RF12_HDR_ACK    0x20
/// RF12 HDR bit mask.
//#define RF12_HDR_MASK   0x1F
/// RF12 SENDER extracted from last packet
//#define RF12_SOURCEID   rf12_hdr & RF12_HDR_MASK

/// RF12 Maximum message size in bytes.
/// Ram needed 
#define RF12_MAXDATA    128
/// Max transmit/receive buffer: 4 header + data + 2 crc bytes
#define RF_MAX          (RF12_MAXDATA + 6)

//frequency bands
#define RF12_315MHZ     0
#define RF12_433MHZ     1
#define RF12_868MHZ     2
#define RF12_915MHZ     3

//Low battery threshold (eg 2v25 = 2.25V)
#define RF12_2v25       0
#define RF12_2v55       3
#define RF12_2v65       4
#define RF12_2v75       5
#define RF12_3v05       8
#define RF12_3v15       9
#define RF12_3v25       10


// enable code compilation for ARSSI readings
#define RF12_ARSSI_ANALOG

// When reading ARSSI from ADC we need to know
// device we're on because values differs from
// one chip to another, can be 
// 300mV to 1000mV
// 450mV to 1150mV
// 600mv to 1300mV
// we need to pass this parameter when we want to set ARSSI Idle value (one time before init)
// ARSSI value
#define RF12_ARSSI_300mV  300
#define RF12_ARSSI_450mV  450
#define RF12_ARSSI_600mV  600
#define RF12_ARSSI_maxmV  700 // max is always ARSSI value + 700mV

#define RF12_ARSSI_MAX        -65   // Arssi maximal value
#define RF12_ARSSI_MIN        -100  // Arssi minimal value
#define RF12_ARSSI_ABOVE_MAX  -64   // Arssi value above maximal value
#define RF12_ARSSI_BELOW_MIN  -101  // Arssi value below minimal value
#define RF12_ARSSI_RECV       -102  // Can't get Arssi because we're not in idle mode
#define RF12_ARSSI_BAD_IDLE   -103  // idle mv def is not in RF12_ARSSI_300mV...RF12_ARSSI_600mV
#define RF12_ARSSI_DISABLED   -104  // Analog RSSI not used 
#define RF12_ARSSI_NB_BYTES   -110  // Can't get Arssi because we do not have enough bytes received
#define RF12_ARSSI_MIN_BYTES  6     // Number of bytes we need to receive to rely on Arssi value

// ARSSI Config bit mask values
#define RF12_ARSSI_ANALOG_PIN_MASK  0x0F
#define RF12_ARSSI_NO_ADC_MASK      0x10

#define RF12_HDR_IDMASK      0x7F
#define RF12_HDR_ACKCTLMASK  0x80
#define RF12_DESTID   (rf12_hdr1 & RF12_HDR_IDMASK)
#define RF12_SOURCEID (rf12_hdr2 & RF12_HDR_IDMASK)

// shorthands to simplify sending out the proper ACK when requested
#define RF12_WANTS_ACK ((rf12_hdr2 & RF12_HDR_ACKCTLMASK) && !(rf12_hdr1 & RF12_HDR_ACKCTLMASK))

// options for RF12_sleep()
#define RF12_SLEEP   0
#define RF12_WAKEUP -1

/// Shorthand for RF12 group byte in rf12_buf.
#define rf12_grp        rf12_buf[0]
/// pointer to 1st header byte in rf12_buf (CTL + DESTINATIONID)
#define rf12_hdr1        rf12_buf[1]
/// pointer to 2nd header byte in rf12_buf (ACK + SOURCEID)
#define rf12_hdr2        rf12_buf[2]

/// Shorthand for RF12 length byte in rf12_buf.
#define rf12_len        rf12_buf[3]
/// Shorthand for first RF12 data byte in rf12_buf.
#define rf12_data       (rf12_buf + 4)

// pin change interrupts are currently only supported on ATmega328's
//#define PINCHG_IRQ 1    // uncomment this to use pin-change interrupts
                          // and set RFM_DEFAULT_IRQ to pin value

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega88) || defined(__AVR_ATmega8__) || defined(__AVR_ATmega88__)
  #define RFM_DEFAULT_IRQ 2   // default IRQ PIN D2 (can be changed by SetIRQ)
  #define SPI_SS          10  // D10 
  #define SPI_MOSI        11  // PB3, pin 17
  #define SPI_MISO        12  // PB4, pin 18
  #define SPI_SCK         13  // PB5, pin 19
#elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)
  #define RFM_DEFAULT_IRQ 2   // default IRQ PIN D2 -> PB2, pin 3(can be changed by SetIRQ)
  #define SPI_SS          4   // PB4, pin 5
  #define SPI_MOSI        5   // PB5, pin 6
  #define SPI_MISO        6   // PB6, pin 7
  #define SPI_SCK         7   // PB7, pin 8
#else
  #error Target not supported for HW Interrupts
#endif

// RF12 command codes
#define RF_RECEIVER_ON  0x82DD
#define RF_XMITTER_ON   0x823D
#define RF_IDLE_MODE    0x820D
#define RF_SLEEP_MODE   0x8205
#define RF_WAKEUP_MODE  0x8207
#define RF_TXREG_WRITE  0xB800
#define RF_RX_FIFO_READ 0xB000
#define RF_WAKEUP_TIMER 0xE000

//RF12 status bits
#define RF_LBD_BIT      0x0400
#define RF_RSSI_BIT     0x0100

// transceiver states, these determine what to do with each interrupt
enum {
  TXCRC1, TXCRC2, TXTAIL, TXDONE, TXIDLE,
  TXRECV,
  TXPRE1, TXPRE2, TXPRE3, TXSYN1, TXSYN2,
};

extern volatile uint8_t rf12_buf[RF_MAX]; // recv/xmit buf, including hdr & crc bytes
class RFM12B
{
  #ifdef RF12_ARSSI_ANALOG
    static volatile uint16_t arssi_idle;  // number of mV read on arssi pin when not receiving 
                                          // depends on chip but usually it is 300, 450 or 600 mV
    static volatile uint16_t arssi;       // number of data bytes in rf12_buf
    static volatile uint8_t arssi_bytes;  // analog rssi values readed
    static volatile uint8_t arssi_config; // Used to indicate no ADC conversion should occurs and Analog pin

  #endif 

  static volatile uint8_t rxfill;           // number of data bytes in rf12_buf
  static volatile int8_t rxstate;           // current transceiver state
  static volatile uint16_t rf12_crc;        // running crc value
  static uint32_t seqNum;                   // encrypted send sequence number
  static uint32_t cryptKey[4];              // encryption key to use
  static long rf12_seq;                     // seq number of encrypted packet (or -1)
  static uint8_t cs_pin;                    // chip select pin
  static uint8_t irq_pin;                   // hardware irq pin ( 2 or 3)
  void (*crypter)(bool);                   // does en-/decryption (null if disabled)
  static uint8_t Byte(uint8_t out);
  static uint16_t XFERSlow(uint16_t cmd);
  static void XFER(uint16_t cmd);
  static void DisableInterrupts(); 
  static void EnableInterrupts();
  static void ConfigureInterrupts();
   
  void SPIInit();
  
	public:
    //constructor
    RFM12B():Data(rf12_data),DataLen(&rf12_buf[3]){}

		volatile static bool noADC; 			// Used to indicate no ADC conversion should occurs
    static uint8_t networkID;         // network group
    static uint8_t nodeID;            // address of this node
    static const byte DATAMAXLEN;
    volatile uint8_t* Data;
    volatile uint8_t* DataLen;
    
    static void InterruptHandler();
    
    void SetCS(uint8_t pin=SPI_SS);
    void SetIRQ(uint8_t irqPin = RFM_DEFAULT_IRQ);
    bool isPresent(uint8_t cspin=SPI_SS, uint8_t irqpin=RFM_DEFAULT_IRQ) ;

    //Defaults: Group: 0xAA=170, transmit power: 0(max), KBPS: 38.3Kbps (air transmission baud - has to be same on all radios in same group)
  	bool Initialize(uint8_t nodeid, uint8_t freqBand, uint8_t groupid=0xAA, uint8_t txPower=0, uint8_t airKbps=0x08, uint8_t lowVoltageThreshold=RF12_2v75);
    void ReceiveStart();
    bool ReceiveComplete();
    bool CanSend();
    uint16_t Control(uint16_t cmd);
		 
    
   #ifdef RF12_ARSSI_ANALOG
    void noRSSI(bool _state);
    void SetRSSI(uint8_t analog_pin = A0, uint16_t _arssi_idle = 0);
    uint16_t getRSSIIdle() { return arssi_idle; }
    uint8_t getRSSIAnalogPin() { return arssi_config & RF12_ARSSI_ANALOG_PIN_MASK ; }
    int8_t ReadARSSI(uint16_t);
   #endif
    
    void SendStart(uint8_t toNodeId, bool requestACK=false, bool sendACK=false);
    void SendStart(uint8_t toNodeId, const void* sendBuf, uint8_t sendLen, bool requestACK=false, bool sendACK=false, uint8_t waitMode=SLEEP_MODE_STANDBY);
    void SendACK(const void* sendBuf = "", uint8_t sendLen=0, uint8_t waitMode=SLEEP_MODE_IDLE);
    void Send(uint8_t toNodeId, const void* sendBuf, uint8_t sendLen, bool requestACK = false, uint8_t waitMode=SLEEP_MODE_STANDBY);
    void SendWait(uint8_t waitMode=0);

    void OnOff(uint8_t value);
    void Sleep(char n);
    void Sleep();
    void Wakeup();
    
    volatile uint8_t * GetData();
    uint8_t GetDataLen(); //how many bytes were received
    uint8_t GetSender();
    uint8_t GetNetworkID() { return rf12_grp; };
    bool LowBattery();
    bool ACKRequested();
    bool ACKReceived(uint8_t fromNodeID=0);
    static void CryptFunction(bool sending);
    void Encrypt(const uint8_t* key, uint8_t keyLen = 16);
    bool CRCPass() { return rf12_crc == 0; }
};

#endif
