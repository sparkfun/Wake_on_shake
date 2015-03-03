/****************************************************************************** <filename>
Wake-on-Shake Neopixel Demo
Joel Bartlett @ SparkFun Electronics
2014
https://github.com/sparkfun/Wake_on_shake

Demo using the Wake-on-Shake to run Neopixels using the Neopixel Library from Adafruit  

Resources:
Adafruit Neopixel Library:
https://github.com/adafruit/Adafruit_NeoPixel
 
This code is beerware; if you see me (or any other SparkFun employee) at the local, and you've found our code helpful, please buy us a round!
Distributed as-is; no warranty is given.
************************************************************/

#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_RGB     Pixels are wired for RGB bitstream
//   NEO_GRB     Pixels are wired for GRB bitstream
//   NEO_KHZ400  400 KHz bitstream (e.g. FLORA pixels)
//   NEO_KHZ800  800 KHz bitstream (e.g. High Density LED strip)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(9, PIN, NEO_GRB + NEO_KHZ800);

int led1 = 5;
int led2 = 9;
int led3 = 10;
int led4 = 11;

int wake = A5;

void setup() 
{
  
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  pinMode(led4, OUTPUT);
  pinMode(wake, OUTPUT);
  
  
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
  delay(100); 
  digitalWrite(wake, HIGH);
  
    for(int i=0;i<=255;i++)
  {
    analogWrite(led1, i);
    delay(5);
  }
    for(int i=255;i>=0;i--)
  {
    analogWrite(led1, i);
    delay(5);
  }
  
  for(int i=0;i<=255;i++)
  {
    analogWrite(led2, i);
    delay(5);
  }
    for(int i=255;i>=0;i--)
  {
    analogWrite(led2, i);
    delay(5);
  }
  
  for(int i=0;i<=255;i++)
  {
    analogWrite(led3, i);
    delay(5);
  }
    for(int i=255;i>=0;i--)
  {
    analogWrite(led3, i);
    delay(5);
  }
  
  for(int i=0;i<=255;i++)
  {
    analogWrite(led4, i);
    delay(5);
  }
    for(int i=255;i>=0;i--)
  {
    analogWrite(led4, i);
    delay(5);
  }
  
  rainbowCycle(20);
  
  digitalWrite(wake, LOW);
}

void loop() 
{
  // Some example procedures showing how to display to the pixels:
 // colorWipe(strip.Color(255, 0, 0), 50); // Red
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
 // colorWipe(strip.Color(0, 0, 255), 50); // Blue
  //rainbow(20);

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
