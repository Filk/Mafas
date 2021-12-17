#include "Arduino.h"
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif

SoftwareSerial mySoftwareSerial(10,11); // RX, TX
DFRobotDFPlayerMini myDFPlayer;

int buttonPin= 9;
int buttonState = 0;
int lastButtonState = 0;
boolean songFinished =true;
boolean songFinishedAlone=false;

int trackNumber=1;
int totalTracks=3;

int volumeValue= 15; //initial volume (0-30)
boolean changeVolume=false;
int contadorSeq =0;
const long intervalChangeVolume=60000; //time (ms) to counter to count. When counts reaches ten, volume drop by one value
unsigned long previousMillisChangeVolume= 0;

#define PIN        5 // On Trinket or Gemma, suggest changing this to 1
#define NUMPIXELS 24 // Popular NeoPixel ring size
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

unsigned long previousMillis = 0; // will store last time LED was updated
const long interval = 1000; // interval at which to blink (milliseconds)

unsigned long previousMillisEnd = 0; // time at which machine was turned on
unsigned long intervalEnd = 1200000; // duration of playing (milliseconds) before it shuts down. 20 minutes
boolean forceEnd=false;

void setup() {
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);

  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));

  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
      Serial.println(F("Unable to begin:"));
      Serial.println(F("1.Please recheck the connection!"));
      Serial.println(F("2.Please insert the SD card!"));
      while(true);
    }
    
  pinMode(buttonPin, INPUT);  
  myDFPlayer.volume(volumeValue);

  #if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
  #endif

  pixels.begin(); // INITIALIZE NeoPixel strip object (REQUIRED)
  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(0, 0, 0));
  }
  pixels.show();
  pixels.clear();
}

void loop() {
  pixels.clear(); // Set all pixel colors to 'off'

  unsigned long currentMillis = millis();
  
  if (changeVolume) {
    volumeValue=volumeValue-1;
    if (volumeValue<0) {
      volumeValue=0;
      forceEnd=true;
    }
    myDFPlayer.volume(volumeValue);
    changeVolume=false;
  }

  buttonState = digitalRead(buttonPin);

  if (buttonState != lastButtonState) {
      if (buttonState == LOW) {
        if (songFinished){
          forceEnd=false;
          intervalEnd = intervalEnd + currentMillis;
          myDFPlayer.play(trackNumber);
          songFinished=false;
          trackNumber++;
          if (trackNumber>totalTracks){
            trackNumber=1;
          }
        }
        else if(!songFinished ) {
          myDFPlayer.stop();
          songFinished=true;
        }
      }
  }

  if (songFinishedAlone) {
    myDFPlayer.play(trackNumber);
    trackNumber++;
    if (trackNumber>totalTracks){
      trackNumber=1;
    }
    songFinishedAlone=false;
    songFinished=false;
  }

  lastButtonState = buttonState;

  if (myDFPlayer.available()) {
    printDetail(myDFPlayer.readType(), myDFPlayer.read());
  }

  if (currentMillis - previousMillis >= interval && !songFinished) {
  // save the last time 
  previousMillis = currentMillis;
  
  pixels.setPixelColor((int) random(0,24), pixels.Color((int) random(10,200), (int) random(10,200), (int) random(10,200)));
  pixels.setPixelColor((int) random(0,24), pixels.Color((int) random(10,200), (int) random(10,200), (int) random(10,200)));
  pixels.setPixelColor((int) random(0,24), pixels.Color((int) random(10,200), (int) random(10,200), (int) random(10,200)));
  pixels.show();   // Send the updated pixel colors to the hardware.
  }
  
  if (currentMillis - previousMillisChangeVolume >= intervalChangeVolume && !songFinished) {
      previousMillisChangeVolume = currentMillis;
      contadorSeq = contadorSeq+1;
      if (contadorSeq==30) {
      changeVolume=true;
      contadorSeq=0;
    }
  }

  if (songFinished) {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    pixels.clear();
  }
  
  if (currentMillis - previousMillisEnd >= intervalEnd || forceEnd) {
    for(int i=0; i<NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 0));
    }
    pixels.show();
    pixels.clear();

    myDFPlayer.stop();
    songFinished=true;
  }
  
  delay(10);
}

void printDetail(uint8_t type, int value){
  switch (type) {
    case DFPlayerPlayFinished:
      songFinished=true;
      songFinishedAlone=true;
      Serial.println("songFinished");
      break;
  }
}
