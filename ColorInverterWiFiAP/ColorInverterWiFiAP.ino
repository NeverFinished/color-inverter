/**
 * 
 */

#include <SPI.h>
#include <WiFiNINA.h>
#include "arduino_secrets.h"
///////please enter your sensitive data in the Secret tab/arduino_secrets.h

#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Adafruit_NeoPixel.h>

#define NEOPIN 5
#define BRIGHTNESS 128 // 64 okay in the dark, 128 okay in sunlight?
#define NUM_PIXELS 5

char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int led =  LED_BUILTIN;
int status = WL_IDLE_STATUS;
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

float red, green, blue;
int hue;

WiFiServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_PIXELS, NEOPIN, NEO_GRB + NEO_KHZ800); // strip object
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);


void setup() {
  Serial.begin(115200);

  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}

  Serial.println("Access Point Web Server");
  pinMode(led, OUTPUT);      // set the LED pin mode

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");    
    while (true); // don't continue 
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  pinMode(NEOPIN, OUTPUT);

  strip.begin();
  strip.setBrightness(BRIGHTNESS); // set brightness

  if (!tcs.begin()) {
    while (true) {
    Serial.println("No TCS34725 found ... check your connection!");
      delay(100);
    }; // halt!
  }

  // by default the local IP address of will be 192.168.4.1
  // you can override it with the following:
  // WiFi.config(IPAddress(10, 0, 0, 1));

  Serial.print("Creating access point named: ");
  Serial.println(ssid);

  status = WiFi.beginAP(ssid, pass);
  if (status != WL_AP_LISTENING) {
    Serial.println("Creating access point failed");
    while (true); // don't continue 
  }

  // wait 10 seconds for connection:
  for (int i = 0; i < 100; ++i) {
    invertColor();
    delay(40);
  }

  Serial.println("Starting TCP Server");

  // start the tcp / web server on port 80
  server.begin();

  // you're connected now, so print out the status
  printWiFiStatus();
}

void loop() {

  // compare the previous status to the current status
  if (status != WiFi.status()) {
    // it has changed update the variable
    status = WiFi.status();
    if (status == WL_AP_CONNECTED) {
      // a device has connected to the AP
      Serial.println("Device connected to AP");
    } else {
      // a device has disconnected from the AP, and we are back in listening mode
      Serial.println("Device disconnected from AP");
    }
  }

  invertColor();

  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type: text/plain");
            client.println();

            // re-use http conn as "streaming protocol"
            for (int i = 0; i < 100; ++i) {
              invertColor();
              client.print("r=");
              client.print((int)red);
              client.print("\tg=");
              client.print((int)green);
              client.print("\tb=");
              client.print((int)blue);
              client.print("\th=");                            
              client.print((int)hue);
              //client.print("\tt=" + (millis() / 100));
              client.println();
              client.flush();
              delay(40);
            }

            client.println();
            // break out of the while loop:
            break; // disconnect
          }
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }

        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    client.stop();
    Serial.println("client disconnected");
  }
}

void printWiFiStatus() {
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}

void invertColor() {
  
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

  Serial.print('\t'); Serial.print(millis() / 100);

  //Serial.print("\tH1:\t"); Serial.print(int(hue1));
  //Serial.print("\tH2:\t"); Serial.print(int(hue2));
  //Serial.print("\tS:\t"); Serial.print(hsv[1]);
  //Serial.print("\tV:\t"); Serial.print(hsv[2]);

  // display same color
  strip.setPixelColor(0, strip.Color(red, green, blue));

  struct RGB_set RGB;
  struct HSV_set HSV;
  RGB.r = red;
  RGB.g = green;
  RGB.b = blue;
  RGB2HSV4(RGB, &HSV);
  if (showLabels) Serial.print("\tH4:"); Serial.print('\t'); Serial.print(int(HSV.h));
  //if (showLabels) Serial.print("\tS4:"); Serial.print('\t'); Serial.print(HSV.s);
  //if (showLabels) Serial.print("\tV4:"); Serial.print('\t'); Serial.print(HSV.v);
  HSV.h = (HSV.h + 180) % 360;
  hue = HSV.h;
  HSV2RGB4(HSV, &RGB);
  //if (showLabels) Serial.print("\tR4:"); Serial.print('\t'); Serial.print(RGB.r);
  //if (showLabels) Serial.print("\tG4:"); Serial.print('\t'); Serial.print(RGB.g);
  //if (showLabels) Serial.print("\tB4:"); Serial.print('\t'); Serial.print(RGB.b);

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
