/* nRF24 Receiver
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

String version = "v0.2";

RF24 radio(D3, D4); // CE, CSN
// SCK = GPIO14 = 5 = D5
// MISO = GPIO12 = 6 = D6
// MOSI = GPIO13 = 7 = D7
// SS = GPIO15 = 16 = D8

const byte address[6] = "00001";

// Servo
#include <Servo.h>
Servo servo;
const int servo_pin = D0; // D1/D2/RST is reserved for Motor Shield
int angle = 90; // max_angl/2

// Motor
#include "WEMOS_Motor.h"
Motor M1(0x30,_MOTOR_A, 1000);
int pwm = 30;

void setup() {

  // Serial
  Serial.begin(115200);
  while (!Serial);
  Serial.println("");
  Serial.println("nRF24 Receiver");  

  // Radio
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MAX);
  radio.setDataRate(RF24_1MBPS);
  radio.setCRCLength(RF24_CRC_8);
  radio.startListening();

   bool goodSignal = radio.testRPD();
   if (radio.available()) {
      Serial.println(goodSignal ? "Strong signal > 64dBm" : "Weak signal < 64dBm" );
      //radio.read(0,0);
   }

  // Servo
  servo.attach(servo_pin);
  servo.write(angle);
  delay(15);
  
  // I2C Motor Scan  
  byte error, address;
  int nDevices;
  /*
  Serial.println("Scanning I2C...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.print(address, HEX);
      Serial.println(" !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
     }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found!!!\n");
  }
  else {
    Serial.println("Done.\n");
    M1.setmotor( _CW, pwm);
  } 
  */
}

void loop() {
  
  if (radio.available()) {
    int payload_size = radio.getPayloadSize();    
    char text[payload_size];
    radio.read(&text, payload_size);

    if (strlen(text) == 0) return;;

    int in_speed, in_heading;   
    int n = sscanf(text, "XCTL:%4u:%4u", &in_speed, &in_heading);
    if (n == 2) {
      Serial.println(n);
      Serial.print(F("in_speed="));
      Serial.println(in_speed);
      
      Serial.print(F("in_heading="));
      Serial.println(in_heading);      
  
      Serial.print("IN:    ");
      Serial.print(text);

      int new_angle = map(in_heading, -180, 180, 0, 120); // (-90) to 180?
      angle = new_angle;
      Serial.print("ANGLE: ");
      Serial.print(angle);
      Serial.println("°");
      servo.write(angle);           
        
      pwm = map(in_speed, 0, 1023, 0, 100);   
      Serial.print("PWM:   ");
      Serial.print(pwm);
      Serial.println(" %");

    } else {
      Serial.println("parse_err");
    }

    if (pwm == 0) {
      M1.setmotor(_STANDBY);
    } else if (pwm > 0) {
      M1.setmotor( _CCW, abs(pwm));      
    } else if (pwm < 0) {
      M1.setmotor( _CW, abs(pwm));
    }
    
    yield();
    delay(15); // let it go...     
    Serial.println("");
  }

  
}

int clamp(int in, int min, int max) {
  int out = in;
  if (in <= min) out = min;
  if (in > min) out = in;
  if (in > max) out = max;
  return out;
}

