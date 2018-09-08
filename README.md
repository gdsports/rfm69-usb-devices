# Wireless USB Keyboard Extender Using RFM69
A long range wireless USB keyboard in two parts.

The USB keyboard plugs into the sender. The sender must have USB host
capabiiity to communicate with the keyboard. The SAMD21 M0 is used for this
project. Every key press and release generates a USB HID (Human Interface
Descriptor) report. The report is a 8 byte data struture which is sent using
the RFM69 radio to the receiver.

The receiver plugs into the computer via a USB port. No other components are
required because the RFM69HCW radio is integrated with the Feather board. The
receiver takes USB HID reports from the RFM69 radio then sends to the computer
over USB.

## USB keyboard receiver
```
915 MHz -> Feather 32u4 RFM69HCW -> USB cable -> Computer
```
The receiver may use the M0 or 32u4 Feather version. The 32u4 was used here for
no other reason than it was available.

* Adafruit Feather 32u4 RFM69HCW Packet Radio

Headers do not need to be installed on the board. An  antenna is required.

Upload the RFM69KbdRFM69Rx.ino sketch. This program reads USB HID reports from
the RFM69 radio then sends them out the USB keyboard device interface to the
computer. The Feather is powered from the computer.

## USB keyboard sender

```
USB         USB OTG Host
Keyboard -> cable/adapter -> Feather M0 RFM69HCW -> 915 MHz
                             GND  USB  Tx
                              |    ^   |
                              |    |   |
                              |    |   v
                             GND  5V   RxD
                             CP2104 USB to serial -> Computer
```

WARNING: The sender must use the M0 Feather version because the M0 has USB host
capability. The 32u4 does not.

* USB keyboard
* USB OTG Host Cable - MicroB OTG male to A female
* Adafruit Feather M0 RFM69HCW Packet Radio
* Adafruit CP2104 Friend - USB to Serial Converter

Headers and breadboard are used to connect to power to the Feather board. An
antenna is required.

Upload the RFM69KbdRFM69Tx.ino sketch. This program reads USB HID reports from
the USB keyboard then sends the USB HID reports to the RFM69 radio.

### Upload mode

Powering the Feather is trickier compared to the receiver. When uploading, the
Feather is powered from the computer. The other 5V power source (see below)
MUST be disconnected.

WARNING: Do not connect more than one power source to the Feather at the same
time.

WARNING: When using USB host mode, the Feather RESET button must be pressed
twice to put the board in upload mode. Automatic upload does not work.

```
Feather M0 RFM69HCW -> USB cable -> Computer
```

### Sender mode

When using the Feather in keyboard sender mode, the Feather must be powered by
a 5V power source connected to its USB pin.  In this case, a CP2104 USB serial
board provides the 5V power as well as USB serial for debug output via the
Serial1 UART Tx pin.

When debugging is completed, the CP2104 board is not needed but a 5V power
supply is still required.

The Feather battery power option is not useful here because Lithium batteries
do not provide 5V.
