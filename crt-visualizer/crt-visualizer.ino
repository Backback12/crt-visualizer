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


const uint16_t samples = 256; // must be power of 2
const double samplingFreq = 20000;  // 20k sample rate
// const double samplingFreq = 50000;  // 50k sample rate

double vReal[samples];
double vImag[samples];
double smooth[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFreq);



unsigned long lastSampleTime = 0;
unsigned int samplingPeriodUs = 1000000 / samplingFreq; // micro s


// Create an instance of the graphics library
ESP_8_BIT_GFX videoOut(true /* = NTSC */, 8 /* = RGB332 color */);

void setup() {
  // Initial setup of graphics library
  videoOut.begin();
}


void loop() {
  // sample audio sig
  for (int i = 0; i < samples; i++) {
    unsigned long now = micros();
    while (micros() - now < samplingPeriodUs);  // wait for sample interval

    int audioSignal = analogRead(34);
    vReal[i] = (double)(audioSignal - 1830);  // seems to center around 1830..... find some way to normalize it?
    vImag[i] = 0;
  }


  // compute FFT
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); //optional?
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude(); // compute magnitudes


  // visualize
  videoOut.waitForFrame();  // wait for next composite frame


  videoOut.fillScreen(0);
  videoOut.setCursor(16, 16);
  videoOut.setTextSize(1);
  videoOut.print("audio_signal");

  for (int i = 1; i < samples / 2; i++) {
    // int magnitude = (int)vReal[i];
    // draw last height
    int height = constrain(map(smooth[i], 0, 5000, 0, 200), 0, 200);
    videoOut.drawFastVLine(i * ((int) 256 / samples * 2),     240 - height, height, 0x80);
    videoOut.drawFastVLine(i * ((int) 256 / samples * 2) + 1, 240 - height, height, 0x80);
    

    int magnitude = max((int) vReal[i], (int) (smooth[i] * 0.65));

    smooth[i] = magnitude;

    // scale height (240)
    height = constrain(map(magnitude, 0, 5000, 0, 200), 0, 200);

    // videoOut.drawFastVLine(i, 240-height, height, 0xFF);

    videoOut.drawFastVLine(i * ((int) 256 / samples * 2),     240 - height, height, 0x03);
    videoOut.drawFastVLine(i * ((int) 256 / samples * 2) + 1, 240 - height, height, 0x03);
  }
}
