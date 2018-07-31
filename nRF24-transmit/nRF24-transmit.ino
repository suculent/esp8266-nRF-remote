/* nRF24 Transmitter
 * Library: TMRh20/RF24, https://github.com/tmrh20/RF24/
 */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

struct CDATA {
  int speed;
  int heading;
  int version;
};


RF24 radio(D3, D4); // CE, CSN

const byte address[6] = "00001";
char message[16] = {0};

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
}

void drawFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Demonstrates the 3 included default sizes. The fonts come from SSD1306Fonts.h file
  // Besides the default fonts there will be a program to convert TrueType fonts into this format
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  display->setFont(ArialMT_Plain_10);
  display->drawString(0 + x, 10 + y, String(message));
  //display->setFont(ArialMT_Plain_16);
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

  int val1 = analogRead(A0);
  int val2 = analogRead(A0);
  int val3 = analogRead(A0);
  int val4 = analogRead(A0);
  int val5 = analogRead(A0);

  int val = (val1 + val2 + val3 + val4 + val5) / 5; // prevent noise

  Serial.println(val);
  
  if ((last_value < val - tolerance) || (last_value > val + tolerance)) {
    last_value = val;
    sprintf(message, "%i", val);  
    if (next_message < millis()) {
      next_message = millis() + 100;
      radio.write(&message, sizeof(message));
      Serial.print("Sending ");
      Serial.println(message);
    }    
  }    

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {    
    delay(remainingTimeBudget);
  }
}
