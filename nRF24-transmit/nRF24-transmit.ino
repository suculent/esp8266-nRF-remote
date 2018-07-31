/* nRF24 Transmitter
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */

String version = "v0.2.1";

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <Encoder.h>

// Encoder
Encoder myEnc(D1, D2); //   avoid using pins with LEDs attached
long encoder_position  = 0;

struct CDATA {
  int speed;
  int heading;
  int version;
};


RF24 radio(D3, D4); // CE, CSN

const byte address[6] = "00001";
char message[32] = {0};

/* SSD1306 support */

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`
#include "OLEDDisplayUi.h"
SSD1306Wire  display(0x3c, D1, D2);
OLEDDisplayUi ui     ( &display );

void msOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {
  display->setTextAlignment(TEXT_ALIGN_RIGHT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(128, 0, String(millis()));
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);  
  display->drawString(0, 0, version);
  //display->setFont(ArialMT_Plain_16);
  display->drawString(0, 12, String(message));
}

void drawFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  
  //
  //display->setFont(ArialMT_Plain_24);
}

FrameCallback frames[] = { drawFrame };
int frameCount = 1;
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;


void setup() {

  // Serial
  Serial.begin(115200);
  Serial.println("nRF24 Transmitter");
  
  // Radio
  radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.stopListening();  

  // Input
  pinMode(A0, INPUT);

  // Display
  ui.setTargetFPS(30);
  ui.disableAllIndicators();
  ui.disableAutoTransition();
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();

  display.flipScreenVertically();
}

int last_value = 0;
int tolerance = 2;
unsigned int next_message = 0;

void loop() {

  // Measure analogue value 
  int val1 = analogRead(A0);
  delay(15);
  int val2 = analogRead(A0);
  delay(15);
  int val3 = analogRead(A0);
  delay(15);
  int val4 = analogRead(A0);
  delay(15);
  int val5 = analogRead(A0);

  int val = (val1 + val2 + val3 + val4 + val5) / 5; // prevent noise
  if ((last_value < val - tolerance) || (last_value > val + tolerance)) {
    last_value = val;        
  }    
  Serial.println(val);

  // Fetch encoder position
  long newPosition = myEnc.read();
  if (newPosition != encoder_position) {
    encoder_position = newPosition;
    Serial.println(encoder_position);
  }
 
  // Send combined values as message over radio 
  if (next_message < millis()) {
    next_message = millis() + 100;
    sprintf(message, "XCTL:%04i:%04i\n", val, 90 + encoder_position);
    radio.write(&message, sizeof(message));
    Serial.print("Sending ");
    Serial.println(message);
  }

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {    
    delay(remainingTimeBudget);
  } 
}
