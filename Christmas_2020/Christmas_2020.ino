/************************** Configuration ***********************************/

#include "config.h"
#include "FastLED.h"
#include "String.h"
#include "colorpalettes.h"

#define NONE 'n'
#define TWINKLE 't'
#define SPARKLE 's'
#define RAINBOW 'r'
#define GLOW 'g'
#define CHRISTMAS 'c'
#define TRADITIONAL 'm'


CRGBPalette16 christmasPalette = CRGBPalette16 (
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    CRGB::Red,
    
    CRGB::Red,
    CRGB::Red,
    CRGB::White,
    CRGB::White,
    
    CRGB::Green,
    CRGB::Green,
    CRGB::Green,
    CRGB::Green,

    CRGB::Green,
    CRGB::Green,
    CRGB::White,
    CRGB::White
);
uint8_t paletteIndex = 0;


/************************ Adafruit *******************************/

// set up the 'mode' feed
AdafruitIO_Feed *effect = io.feed("Effect");
AdafruitIO_Feed *power = io.feed("Power");
AdafruitIO_Feed *color = io.feed("Color");

byte re = 0x00;
byte gr = 0x00;
byte bl = 0x00;

char mode = NONE;

/************************ LED Defines Info *******************************/

#define NUM_LEDS 150
CRGB leds[NUM_LEDS];
#define PIN 2 

bool fadeIn = true;
int k = 0;

String pwr = "";
String fx = "";

int newColor = LOW;
int lastUpdate = 0;


void setup() {
    //Light Setup
  FastLED.addLeds<WS2811, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  setAll(0x00, 0x00, 0x00);

  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  Serial.print("Connecting to Adafruit IO");

  // start MQTT connection to io.adafruit.com
  io.connect();

  // set up a message handler for the count feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  effect->onMessage(handleEffect);
  power->onMessage(handlePower);
  color->onMessage(handleColor);

  // wait for an MQTT connection
  // NOTE: when blending the HTTP and MQTT API, always use the mqttStatus
  // method to check on MQTT connection status specifically
  while(io.mqttStatus() < AIO_CONNECTED) {
    Serial.print(".");
    //Serial.println(io.mqttStatus());
    delay(500);
  }

  // Because Adafruit IO doesn't support the MQTT retain flag, we can use the
  // get() function to ask IO to resend the last value for this feed to just
  // this MQTT client after the io client is connected.
  //effect->get();

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  effect -> get();
  power -> get();
  color -> get();



}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();
//  Serial.println(effect->value());

run();
 
  

  // Because this sketch isn't publishing, we don't need
  // a delay() in the main program loop.

}

void run(){
  if(pwr.equals("on")){
    Serial.println("Power On");
    Serial.print("Red: ");
    Serial.print(re);
    Serial.print("   Green: ");
    Serial.print(gr);
    Serial.print("    Blue: ");
    Serial.print(bl);

    switch (mode){
    case TWINKLE:
      SnowSparkle(re, gr, bl, 20, random(100,1000));
      break;
    case GLOW:
      FadeInOut(re, gr, bl);
      break;
    case SPARKLE:
      meteorRain(re,gr,bl,10, 64, true, 30);
      break;
    case CHRISTMAS:
      EVERY_N_MILLISECONDS(50){
        //Switch on an LED at random, choosing a random color from the palette
        leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(christmasPalette, random8(), 255, LINEARBLEND);
      }
       // Fade all LEDs down by 1 in brightness each time this is called
       fadeToBlackBy(leds, NUM_LEDS, 1);
       showStrip();
      break;
    case RAINBOW:
      EVERY_N_MILLISECONDS(50){
        //Switch on an LED at random, choosing a random color from the palette
        leds[random8(0, NUM_LEDS - 1)] = ColorFromPalette(RainbowColors_p, random8(), 255, LINEARBLEND);
      }
       // Fade all LEDs down by 1 in brightness each time this is called
       fadeToBlackBy(leds, NUM_LEDS, 1);
       showStrip();;
      break;
    case TRADITIONAL:
      setAll(0x9a, 0x6a, 0x21);
      break;
    case NONE:
    default:
      setAll(re,gr,bl);
    }
  }
  else{
    //Serial.println("Power Off");
    setAll(0x00,0x00,0x00);
  }
}
/************************ Handle Message *******************************/
// this function is called whenever a 'mode' message
// is received from Adafruit IO. it was attached to
// the mode feed in the setup() function above.
void handlePower(AdafruitIO_Data *data) {
  
  Serial.print("received <- ");
  Serial.println(data->value());
  pwr = data -> value();
  pwr.toLowerCase();
  
}

void handleEffect(AdafruitIO_Data *data) {
  
  Serial.print("received <- ");
  Serial.println(data->value());
  fx = data -> value();
  fx.toLowerCase();

  if(fx.equals("twinkle")) mode = TWINKLE;
  else if(fx.equals("sparkle")) mode = SPARKLE;
  else if(fx.equals("glow")) mode = GLOW;
  else if(fx.equals("christmas")) mode = CHRISTMAS;
  else if(fx.equals("rainbow")){
    mode = RAINBOW;
    setAll(0x00, 0x00, 0x00);
  }
  else if(fx.equals("traditional")){
    mode = TRADITIONAL;
    setAll(0x00, 0x00, 0x00);
  }
  else mode = NONE; 
}

void handleColor(AdafruitIO_Data *data) {
  
  Serial.print("received <- ");
  Serial.println(data->value());
  //hue = data -> value();
  re = data -> toRed();
  gr = data -> toGreen();
  bl = data -> toBlue();
  
}


/************************ Light Functions *******************************/
void showStrip() {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.show();
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   FastLED.show();
 #endif
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
   // NeoPixel
   strip.setPixelColor(Pixel, strip.Color(red, green, blue));
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H 
   // FastLED
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
 #endif
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}

/************************ Effects *******************************/
//Twinkle
//  SnowSparkle(0x10, 0x10, 0x10, 20, random(100,1000));
void SnowSparkle(byte red, byte green, byte blue, int SparkleDelay, int SpeedDelay) {
  float r,g,b;
  r = 0.1*red;
  g = 0.1*green;
  b = 0.1*blue;
  setAll(r,g,b);
 
  int Pixel = random(NUM_LEDS);
  setPixel(Pixel,red,green,blue);
  showStrip();
  delay(SparkleDelay);
  setPixel(Pixel,r,g,b);
  showStrip();
  delay(SpeedDelay);
}

//Pixie Dust
//  meteorRain(0xff,0xff,0xff,10, 64, true, 30);
void meteorRain(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
 
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
   
   
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
   
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      }
    }
   
    showStrip();
    delay(SpeedDelay);
    
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
   
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
   
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}

//Glow
//  FadeInOut(0xff, 0x77, 0x00);
void FadeInOut(byte red, byte green, byte blue){
  float r, g, b;
    r = (k/256.0)*red;
    g = (k/256.0)*green;
    b = (k/256.0)*blue;
    setAll(r,g,b);
    
   if(fadeIn){
    k += 1;
    if(k==256)
      fadeIn = false;
   }
   else{
    k -= 1;
    if(k==0)
      fadeIn = true;
   }
  
}
