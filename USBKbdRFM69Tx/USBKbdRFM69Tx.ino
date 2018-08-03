/*
 * Keyboard Wireless Extender Example -- Sender
 * Intercept USB keyboard HID report then send it via RFM69 raw mode.
 * Based on an example included in Adafruit's fork of the RadioHead library.
 */

//#define DEBUG_KEYBOARD_RAW 1

#include <SPI.h>
#include <RH_RF69.h>
#include <RHReliableDatagram.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 915.0

// Where to send packets to!
#define DEST_ADDRESS   1
// change addresses for each client board, any number :)
#define MY_ADDRESS     2


#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined(ARDUINO_SAMD_FEATHER_M0)  // Adafruit Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           LED_BUILTIN
  #ifdef SERIAL_PORT_MONITOR
    #undef SERIAL_PORT_MONITOR
  #endif
  #define SERIAL_PORT_MONITOR Serial1
#elif defined(ARDUINO_ITSYBITSY_M0)   // Adafruit ItsyBitsy M0
  #define RFM69_CS      10
  #define RFM69_INT     7
  #define RFM69_RST     9
  #define LED           LED_BUILTIN
  #ifdef SERIAL_PORT_MONITOR
    #undef SERIAL_PORT_MONITOR
  #endif
  #define SERIAL_PORT_MONITOR Serial1
#elif defined(ARDUINO_SAMD_ZERO)      // Arduino Zero
  #define RFM69_CS      4
  #define RFM69_INT     3
  #define RFM69_RST     2
  #define LED           LED_BUILTIN
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  //
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_IRQ     15   // "B"
  #define RFM69_RST     16   // "D"
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

/* Teensy 3.x w/wing
#define RFM69_RST     9   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     4    // "C"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/

/* WICED Feather w/wing
#define RFM69_RST     PA4     // "A"
#define RFM69_CS      PB4     // "B"
#define RFM69_IRQ     PA15    // "C"
#define RFM69_IRQN    RFM69_IRQ
*/

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram rf69_manager(rf69, MY_ADDRESS);

#include <hidboot.h>

class KeyboardRaw : public KeyboardReportParser {
public:
  KeyboardRaw(USBHost &usb) : hostKeyboard(&usb) {
    hostKeyboard.SetReportParser(0, this);
  };

  void Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf);

private:
  HIDBoot<HID_PROTOCOL_KEYBOARD> hostKeyboard;
};

void KeyboardRaw::Parse(HID *hid, uint32_t is_rpt_id, uint32_t len, uint8_t *buf)
{
#ifdef DEBUG_KEYBOARD_RAW
  SERIAL_PORT_MONITOR.print("KeyboardRaw::Parse");
  // Show USB HID keyboard report
  for (uint8_t i = 0; i < len ; i++) {
    SERIAL_PORT_MONITOR.print(' '); SERIAL_PORT_MONITOR.print(buf[i], HEX);
  }
  SERIAL_PORT_MONITOR.println();
#endif

  // Call parent/super method
  KeyboardReportParser::Parse(hid, is_rpt_id, len, buf);

  // On error - return
  if (buf[2] == 1)
    return;

  if (len == 8) {
    rf69_manager.sendtoWait(buf, len, DEST_ADDRESS);
  }
}

// Initialize USB Controller
USBHost usb;

// Attach keyboard controller to USB
KeyboardRaw keyboard(usb);

void setup()
{
  SERIAL_PORT_MONITOR.begin( 115200 );
  //while (!SERIAL_PORT_MONITOR); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
  SERIAL_PORT_MONITOR.println("KeyboardRaw Program started");

  if (usb.Init() == -1)
    SERIAL_PORT_MONITOR.println("USB host controller did not start.");

  delay( 20 );

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  SERIAL_PORT_MONITOR.println("Feather RFM69 TX Test!");
  SERIAL_PORT_MONITOR.println();

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69_manager.init()) {
    SERIAL_PORT_MONITOR.println("RFM69 radio init failed");
    while (1);
  }
  SERIAL_PORT_MONITOR.println("RFM69 radio init OK!");
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    SERIAL_PORT_MONITOR.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  rf69.setEncryptionKey(key);

  pinMode(LED, OUTPUT);

  SERIAL_PORT_MONITOR.print("RFM69 radio @");
  SERIAL_PORT_MONITOR.print((int)RF69_FREQ);
  SERIAL_PORT_MONITOR.println(" MHz");
}

void loop()
{
  // Process USB tasks
  usb.Task();
}

