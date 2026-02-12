/*

Example for ESP_8_BIT color composite video generator library on ESP32.
Connect GPIO25 to signal line, usually the center of composite video plug.

GFX Hello World

This demonstrates using the ESP_8_BIT_GFX class, which inherits from the
Adafruit GFX base class to deliver an easy to use graphics API. Draws two
rectangles that cycle around the border of the screen. The amount of corners
cut off from these rectangle show the amount of overscan on a particular
screen. In the middle of two rectangles are a bit of text drawn using
Adafruit GFX print() API.

Copyright (c) Roger Cheng

MIT License

Permission is herebyc:\Users\cepag\Desktop\Electronics\PROJECTS\crt-visualizer\README.md granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/


/*
Composite video resolution: 255x240


*/


// Import composite video library
#include <ESP_8_BIT_GFX.h>

// fft optimizations (define before import)
// https://github.com/kosme/arduinoFFT/wiki/optimizations
#define USE_AVR_PROGMEM
#define FFT_SPEED_OVER_PRECISION  // replaces most divisions with multiplications by a reciprocal value
#define FFT_SQRT_APPROXIMATION 

#include <arduinoFFT.h>

#include "setting.h"
#include <Button2.h>
#include "CRTEffects.h"



#define POT_PIN 33
// #define AUDIO_PIN 34
#define AUDIO_PIN 35
#define BUTT_PIN 32
//#define VIDEO_PIN 25  // pre-defined in lib

const uint16_t samples = 128; // must be power of 2
const double samplingFreq = 20000;  // 20k sample rate
// const double samplingFreq = 50000;  // 50k sample rate

double vReal[samples];
double vImag[samples];
double smooth[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFreq);



unsigned long lastSampleTime = 0;
unsigned int samplingPeriodUs = 1000000 / samplingFreq; // micro s



Button2 button;

// Create an instance of the graphics library
ESP_8_BIT_GFX videoOut(true /* = NTSC */, 8 /* = RGB332 color */);



unsigned long lastAnimationTime = 0;
byte animX = 0;
byte animY = 0;

unsigned long settingAnimTimer = 0; // if > millis() show anim. If < millis(), hide anim
bool settingActive;
int settingLockVal;  // locked value, move += 5% to show setting
int potValue;
byte colour = 0x03;

// settings!
const int NUM_SETTINGS = 2;
Setting settings[NUM_SETTINGS] = {
  Setting("Hue", 4096),
  Setting("Log Exponent", 10),
};
int activeSetting = 0;


void settingClickHandler(Button2& btn) {
  settings[activeSetting].deactivate();
  activeSetting = (activeSetting + 1) % NUM_SETTINGS;
  settings[activeSetting].activate();
  
  // set animation
  settingActive = true;
  settingAnimTimer = millis() + 3000;
}



CRTEffects effects(&videoOut, samples);
int currentEffect = 0;



void setup() {
  // Initial setup of graphics library
  videoOut.begin();

  potValue = analogRead(POT_PIN);
  settingLockVal = potValue;
  settingActive = false;
  

  // test images for corners!!!
  // while (true) {
  //   videoOut.waitForFrame();


  //   videoOut.fillScreen(0x1C);  // fill green
  //   int offsetX = 16;
  //   int offsetY = 16;
  //   videoOut.fillRect(offsetX, offsetY, 255 - 2*offsetX, 240 - 2*offsetY, 0xE0);  // fill red
  // }
  // safe border margin is 16px?

  // setup button
  button.begin(BUTT_PIN);
  button.setClickHandler(settingClickHandler);
  //button.setDoubleClickHandler();
  //button.setLongClickHandler(); or button.setLongClickDetectedHandler();
  //button.setLongClickTime(ms)
  //button.setDoubleClickTime(ms)


  // s_hue.activate();
  // s_exp.activate();
  settings[activeSetting].activate();

  Serial.begin(115200);
}


void loop() {
  // read button
  button.loop();

  // sample audio sig
  for (int i = 0; i < samples; i++) {
    unsigned long now = micros();
    while (micros() - now < samplingPeriodUs);  // wait for sample interval

    // int audioSignal = analogRead(AUDIO_PIN);
    vReal[i] = (double)(analogRead(AUDIO_PIN) - 1830);  // seems to center around 1830..... find some way to normalize it?
    vImag[i] = 0;
  }


  // compute FFT
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); //optional?
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude(); // compute magnitudes


  // visualize
  videoOut.waitForFrame();  // wait for next composite frame
  videoOut.fillScreen(0);

  /*
  if (millis() > lastAnimationTime + 10000) {
    // every 10 seconds, update position (prevents burn in?)
    lastAnimationTime = millis();
    animX = random(0, 200);
    animY = random(0, 30);
  }
  // videoOut.setCursor(16, 16);
  videoOut.setCursor(animX, animY);
  videoOut.setTextColor(colour);
  videoOut.setTextSize(1);
  videoOut.print("audio_signal");

  int lastX = 16 - 2;
  int lastY = 240;
  */
  
  // int currentEffect;
  if (millis() > lastAnimationTime + 10000) {
    lastAnimationTime = millis();
    // currentEffect = (currentEffect + 1) % 6;
    currentEffect++;

    Serial.print("Current effect: #");
    Serial.println(currentEffect);
  }
  effects.updateData(vReal, colour);

  switch(currentEffect) {
    case 0: effects.colouredBarsBottom(); break;
    case 1: effects.colouredBarsCenter(); break;
    case 2: effects.whiteLineTick(); break;
    case 3: effects.logColouredBarsTick(2.5); break;
    case 4: effects.pulsingOrbs(); break;
    case 5: effects.nebulaTunnel(); break;
    case 6: effects.sonicRings(); break;
    case 7: effects.oceanWave(); break;
    case 8: effects.bassTongue(); break;
    case 9: effects.radarScan(); break;
    default: currentEffect = 0;
  }


  // handle settings and stuff
  potValue = potValue * 0.6 + analogRead(POT_PIN) * 0.4;
  // s_exp.setValueFromPot(potValue);
  settings[activeSetting].setValueFromPot(potValue);

  
  // colour = hsvToRGB332(255 * potValue / 4095, 255, 255);
  // colour = hsvToRGB332(255 * s_exp.getRawValue() / 4095, 255, 255);
  colour = hsvToRGB332(255 * settings[activeSetting].getRawValue() / 4095, 255, 255);
  
  if (abs(potValue - settingLockVal) > 205) { // changed by >5% (5% of 4096 is 204.8)
    settingAnimTimer = millis() + 3000; // 3 seconds hold
    settingLockVal = potValue;

    if (!settingActive) {
      settingActive = true;
    }
  }

  if (settingActive && millis() > settingAnimTimer) {
    settingActive = false;
  }

  if (settingActive) {
    // display setting
    videoOut.setCursor(32, 167);
    videoOut.setTextColor(0x1C);
    videoOut.setTextSize(1);
    // videoOut.print("Hue");
    // videoOut.print(s_exp.getName());
    videoOut.print(settings[activeSetting].getName());
    // videoOut.print((int) 50 * potValue / 4095); //RGB332
    // videoOut.fillRect(32, 176, 191 * potValue / 4095, 16, 0b00011000);
    // videoOut.fillRect(32, 176, 191 * s_exp.getRawValue() / 4095, 16, 0b00011000);
    videoOut.fillRect(32, 176, 191 * settings[activeSetting].getRawValue() / 4095, 16, 0b00011000);
    videoOut.drawRect(32, 176, 191, 16, 0x1C);  
  }
}




uint8_t hsvToRGB332(uint8_t h, uint8_t s, uint8_t v) {
  uint8_t region = h / 43; // 0–5
  uint8_t remainder = (h - (region * 43)) * 6;

  uint8_t p = (v * (255 - s)) >> 8;
  uint8_t q = (v * (255 - ((s * remainder) >> 8))) >> 8;
  uint8_t t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

  uint8_t r, g, b;
  switch (region) {
    case 0: r = v; g = t; b = p; break;
    case 1: r = q; g = v; b = p; break;
    case 2: r = p; g = v; b = t; break;
    case 3: r = p; g = q; b = v; break;
    case 4: r = t; g = p; b = v; break;
    default: r = v; g = p; b = q; break;
  }

  // Now reduce to 3-3-2 bits for RGB332
  uint8_t r3 = r >> 5;      // 3 bits
  uint8_t g3 = g >> 5;      // 3 bits
  uint8_t b2 = b >> 6;      // 2 bits

  return (r3 << 5) | (g3 << 2) | b2;
}