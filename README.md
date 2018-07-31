# esp8266-nRF-remote

Pet project involving 2x nRF24 radio connected over SPI to Wemos D1 Mini with Wemos Motor Shield, motor and servo; and Wemos D1 + OLED with KY-041 knob and fader.

## Transmitter

* ESP8266 on Wemos board with OLED display
* nRF24
* KY-041 stepper
* Fader

### Fader Connection

GND -> GND
VCC -> 3V VCC (A0 has 1.0V limit and this is a voltage divider)
OUT -> A0

### KY-041 Stepper Connection

> TODO: FIX this, stepper should not be on D1/D2 which is breaking OLED display on Transmitter!
> Problem: D1/D2 is used by OLED, D8 is used by SPI lib as SS, D0 is empty but wakeup/upload blocker... all that remains is RX(3)/TX(1)!
> Possible solution: adjust nRF library to use MISO/MOSI/SCLK/SS from RAM bus (CLK, SDO, CMD, SD1, SD2, SD3 pins are unknown)

CLK -> D1
DT -> D2

### nRF24 Connection for Transmitter

CE -> D3
CSN -> D4
SCK -> D5
MO -> D7
MI -> D6

## Receiver

* ESP8266 on Wemos D1 mini Pro board
* nRF24
* Small Servo
* Wemos Motor Shield
* Motor (6V)

### Servo Connection

CTL -> D0
GNC -> GND
VCC -> 5V VCC

### Motor Connection

Motor is connected to shaft A of Wemos Motor Shield. Optionally, battery shield can be attached to cover monitoring, controls, communication and optional GPS services while the main engine power cell is depleted.

Motor and Servo should have its own power cell (separate from the main board cell) with single grounding for both.

### nRF24 Connection for Transmitter

CE->D3
CSN->D4
SCK->D5
MO->D7
MI->D6
