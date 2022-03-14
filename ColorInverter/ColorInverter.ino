

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
const uint8_t PROGMEM gammaTable[] = { _G3_ _G3_ _G3_ _G3_ }; // 256 items

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
  
  Serial.println("Hello World!");

    pinMode(NEOPIN, OUTPUT);

      strip.begin();

      strip.setBrightness(BRIGHTNESS); // set brightness


  if (tcs.begin()) {
    //Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connection!");
    while (1); // halt!
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

  // display same color
  strip.setPixelColor(0, strip.Color(red, green, blue));
        
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
  strip.setPixelColor(2, strip.Color(RGB.r, RGB.g, RGB.b));
  strip.setPixelColor(3, strip.Color(RGB.r, RGB.g, RGB.b));
  strip.setPixelColor(4, strip.Color(RGB.r, RGB.g, RGB.b));

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

// HSV (hue-saturation-value) to RGB function used for the next two modes.
uint32_t hsv2rgb(int32_t h, uint8_t s, uint8_t v) {
  uint8_t n, r, g, b;

  // Hue circle = 0 to 1530 (NOT 1536!)
  h %= 1530;           // -1529 to +1529
  if(h < 0) h += 1530; //     0 to +1529
  n  = h % 255;        // Angle within sextant; 0 to 254 (NOT 255!)
  switch(h / 255) {    // Sextant number; 0 to 5
   case 0 : r = 255    ; g =   n    ; b =   0    ; break; // R to Y
   case 1 : r = 254 - n; g = 255    ; b =   0    ; break; // Y to G
   case 2 : r =   0    ; g = 255    ; b =   n    ; break; // G to C
   case 3 : r =   0    ; g = 254 - n; b = 255    ; break; // C to B
   case 4 : r =   n    ; g =   0    ; b = 255    ; break; // B to M
   default: r = 255    ; g =   0    ; b = 254 - n; break; // M to R
  }

  uint32_t v1 =   1 + v; // 1 to 256; allows >>8 instead of /255
  uint16_t s1 =   1 + s; // 1 to 256; same reason
  uint8_t  s2 = 255 - s; // 255 to 0

  r = ((((r * s1) >> 8) + s2) * v1) >> 8;
  g = ((((g * s1) >> 8) + s2) * v1) >> 8;
  b = ((((b * s1) >> 8) + s2) * v1) >> 8;

  // gamma correction
  r = pgm_read_byte(&gammaTable[r]);
  g = pgm_read_byte(&gammaTable[g]);
  b = pgm_read_byte(&gammaTable[b]);

  return ((uint32_t)r << 16) | ((uint16_t)g << 8) | b;
}
