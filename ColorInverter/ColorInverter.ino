/**
 * Target Platform Arduino Nano 33 IoT
 * TCS34725 Color sensor on I2C
 * Some NeoPixels on NEOPIN
 */

#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>

#define NEOPIN 5
#define BRIGHTNESS 128 // 64 okay in the dark, 128 okay in sunlight?
#define NUM_PIXELS 5

#define _GAMMA_ 2.6
const int _GBASE_ = __COUNTER__ + 1; // Index of 1st __COUNTER__ ref below
#define _G1_ pow((__COUNTER__ - _GBASE_) / 255.0, _GAMMA_) * 255.0 + 0.5,
#define _G2_ _G1_ _G1_ _G1_ _G1_ _G1_ _G1_ _G1_ _G1_ // Expands to 8 items
#define _G3_ _G2_ _G2_ _G2_ _G2_ _G2_ _G2_ _G2_ _G2_ // Expands to 64 items
//const uint8_t PROGMEM gammaTable[] = { _G3_ _G3_ _G3_ _G3_ }; // 256 items

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIN, NEO_GRB + NEO_KHZ800); // strip object

// our RGB -> eye-recognized gamma color
byte gammatable[256];
float hsv[3];
double hsv3[3];
float rgb[3];
boolean showReal = false;
boolean showLabels = false;

struct RGB_set {
 unsigned char r;
 unsigned char g;
 unsigned char b;
} RGB_set;
 
struct HSV_set {
 signed int h;
 unsigned char s;
 unsigned char v;
} HSV_set;

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

void setup() {
  Serial.begin(115200);
  
  pinMode(NEOPIN, OUTPUT);

  strip.begin();

  strip.setBrightness(BRIGHTNESS); // set brightness

  if (!tcs.begin()) {
    while (true) {
    Serial.println("No TCS34725 found ... check your connection!");
      delay(100);
    }; // halt!
  }

  // thanks PhilB for this gamma table!
  // it helps convert RGB colors to what humans see
  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;
    gammatable[i] = x;
  }
}

// The commented out code in loop is example of getRawData with clear value.
// Processing example colorview.pde can work with this kind of data too, but It requires manual conversion to 
// [0-255] RGB value. You can still uncomments parts of colorview.pde and play with clear value.
void loop() {
  float red, green, blue;
  
  tcs.setInterrupt(false);  // turn on LED

  delay(60);  // takes 50ms to read

  tcs.getRGB(&red, &green, &blue);

  // white balance correction:
  red *= 1.25;
  green /= 1.1;
  
  tcs.setInterrupt(true);  // turn off LED

  if (showLabels) { Serial.print("R:"); Serial.print('\t'); } Serial.print(int(red)); 
  if (showLabels) Serial.print("\tG:"); Serial.print('\t'); Serial.print(int(green)); 
  if (showLabels) Serial.print("\tB:"); Serial.print('\t'); Serial.print(int(blue));

  rgb2hsv2(256.0 / red, 256.0 / green, 256.0 / blue, hsv);

  int hue1 = (int(1530 * hsv[0])) % 1530; // normed to 1530
  int hue2 = (int(360.0 * hsv[0]) + 180) % 360; // normed to 0..360

  //Serial.print("\tH1:\t"); Serial.print(int(hue1));
  //Serial.print("\tH2:\t"); Serial.print(int(hue2));
  //Serial.print("\tS:\t"); Serial.print(hsv[1]);
  //Serial.print("\tV:\t"); Serial.print(hsv[2]);

  // display same color:
  // strip.setPixelColor(0, strip.Color(red, green, blue));
        
  //strip.setPixelColor(1, hsv2rgb(hue1, int(255.0 * hsv[1]), 255));

  hsv2rgb2(hsv[0], hsv[1], 1.0, rgb);
  int r = int(255.0 * rgb[0]);
  int g = int(255.0 * rgb[1]);
  int b = int(255.0 * rgb[2]);        
  //strip.setPixelColor(2, strip.Color(r, g, b));

  // something is totally wrong here ... flickers around:     
  //rgb2hsv3(1.0 * red / 256.0, 1.0 * green / 256.0, 1.0 * blue / 256.0, hsv3);    
  //Serial.print("\tH3:\t"); Serial.print(int(hsv3[0])); 
  //double hsv180 = 180.0 + hsv3[0];
  //while (hsv180 >= 360.0) hsv180 -= 360.0;
  //strip.setPixelColor(3, hsv2rgb3(hsv180, hsv3[1], hsv3[2]));

  struct RGB_set RGB;
  struct HSV_set HSV;
  RGB.r = red;
  RGB.g = green;
  RGB.b = blue;
  RGB2HSV4(RGB, &HSV);
  if (showLabels) Serial.print("\tH4:"); Serial.print('\t'); Serial.print(int(HSV.h));
  if (showLabels) Serial.print("\tS4:"); Serial.print('\t'); Serial.print(HSV.s);
  if (showLabels) Serial.print("\tV4:"); Serial.print('\t'); Serial.print(HSV.v);
  HSV.h = (HSV.h + 180) % 360;
  HSV2RGB4(HSV, &RGB);
  if (showLabels) Serial.print("\tR4:"); Serial.print('\t'); Serial.print(RGB.r);
  if (showLabels) Serial.print("\tG4:"); Serial.print('\t'); Serial.print(RGB.g);
  if (showLabels) Serial.print("\tB4:"); Serial.print('\t'); Serial.print(RGB.b);

  // direct conversion has never enough saturation
  // strip.setPixelColor(1, strip.Color(RGB.r, RGB.g, RGB.b));

  // save values
  int sat = HSV.s;
  int val = HSV.v;

  // saturation = full
  HSV.s = 255;
  HSV2RGB4(HSV, &RGB);
  //strip.setPixelColor(2, strip.Color(RGB.r, RGB.g, RGB.b));

  // inverse value (sat=full from previous) // BEST - choose this
  HSV.v = 255 - val;
  HSV2RGB4(HSV, &RGB);
  for (int i = 0; i < NUM_PIXELS; ++i) {
    strip.setPixelColor(i, strip.Color(RGB.r, RGB.g, RGB.b));
  }

  // inverse sat and value - okay, but often not saturated enough when "full" colors are the input
  //HSV.s = 255 - sat;
  //HSV.v = 255 - val;
  //HSV2RGB4(HSV, &RGB);
  //strip.setPixelColor(4, strip.Color(RGB.r, RGB.g, RGB.b));
  
  // zu krass, remove
  //HSV.v = 255;
  //HSV2RGB4(HSV, &RGB);
  //strip.setPixelColor(5, strip.Color(RGB.r, RGB.g, RGB.b));

  // FIXME: adjust sat? by relative delta of RGB...?
      
  strip.show();

  Serial.println();
}
