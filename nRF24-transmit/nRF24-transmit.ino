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

const int button_pin = D8;

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
  pinMode(button_pin, INPUT);

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
int mid_point = 90;

int lowpass_size = 8;
int lowpass_pos = 0;
double x[lowpass_size] = {0,0,0,0,0,0,0,0};

double low_pass_avg(double input, int points = lowpass_size) {
    double sum = 0;
    for (int i = points - 1; i < 0; --i) {
        x[i] = x[i-1];
        sum += x[i];
    }
    sum += input;
    x[0] = input;
    return sum / points;
}

void loop() {

  int average = low_pass_avg(analogRead(A0), lowpass_size);
  Serial.print(average);
  Serial.println(";");

  Serial.print("Measuring analog sequence: ");
  // Measure analogue value
  int val1 = analogRead(A0);
  Serial.print(val1);
  Serial.print(";");
  int val2 = analogRead(A0);
  Serial.print(val2);
  Serial.print(";");
  int val3 = analogRead(A0);
  Serial.print(val3);
  Serial.print(";");
  int val4 = analogRead(A0);
  Serial.print(val4);
  Serial.print(";");
  int val5 = analogRead(A0);
  Serial.println(val5);

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

  // Store zero position if pressed
  bool newState = digitalRead(button_pin);
  if (newState == gpio.LOW) { // should be grounded when pressed
    mid_point = 90 - encoder_position; // should have +/-90 DOF (0-180 servo range); this resets mid-point
    Serial.print("Saving zero position: ");
    Serial.println(mid_point);
  }

  // Send combined values as message over radio
  if (next_message < millis()) {
    next_message = millis() + 100;
    sprintf(message, "XCTL:%04i:%04i\n", val, mid_point + encoder_position);
    radio.write(&message, sizeof(message));
    Serial.print("Sending ");
    Serial.println(message);
  }

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    delay(remainingTimeBudget);
  }
}
