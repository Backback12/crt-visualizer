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

#include <ESP_8_BIT_GFX.h>


// fft optimizations
#define USE_AVR_PROGMEM
#define FFT_SPEED_OVER_PRECISION
#define FFT_SQRT_APPROXIMATION

#include <arduinoFFT.h>


const uint16_t samples = 256; // must be power of 2
const double samplingFreq = 20000;  // 10k sample rate
// const double samplingFreq = 50000;  // 10k sample rate

double vReal[samples];
double vImag[samples];
double smooth[samples];

ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFreq);
// ArduinoFFT<double> FFT = ArduinoFFT<double>(vReal, vImag, samples, samplingFreq, true);



unsigned long lastSampleTime = 0;
unsigned int samplingPeriodUs = 1000000 / samplingFreq; // micro s


// Create an instance of the graphics library
ESP_8_BIT_GFX videoOut(true /* = NTSC */, 8 /* = RGB332 color */);

void setup() {
  // Initial setup of graphics library
  videoOut.begin();
  
}

// int buffer[255];
// int counter;

void loop() {
  /*
  // shift
  for (int i = 255 - 1; i > 0; i--) {
    buffer[i] = buffer[i-1];
  }
  // push
  buffer[0] = analogRead(34);

  counter++;
  if (counter == 255) {
  // Wait for the next frame to minimize chance of visible tearing
  videoOut.waitForFrame();

  // Get the current time and calculate a scaling factor
  // unsigned long time = millis();
  // float partial_second = (float)(time % 1000)/1000.0;

  // Use time scaling factor to calculate coordinates and colors
  // uint8_t movingX = (uint8_t)(255*partial_second);
  // uint8_t invertX = 255-movingX;
  // uint8_t movingY = (uint8_t)(239*partial_second);
  // uint8_t invertY = 239-movingY;

  // uint8_t cycle = colorCycle[(uint8_t)(17*partial_second)];
  // uint8_t invertC = 0xFF-cycle;

  // Clear screen
  videoOut.fillScreen(0);

  // Draw one rectangle
  // videoOut.drawLine(movingX, 0,       255,     movingY, cycle);
  // videoOut.drawLine(255,     movingY, invertX, 239,     cycle);
  // videoOut.drawLine(invertX, 239,     0,       invertY, cycle);
  // videoOut.drawLine(0,       invertY, movingX, 0,       cycle);

  // Draw a rectangle with inverted position and color
  // videoOut.drawLine(invertX, 0,       255,     invertY, invertC);
  // videoOut.drawLine(255,     invertY, movingX, 239,     invertC);
  // videoOut.drawLine(movingX, 239,     0,       movingY, invertC);
  // videoOut.drawLine(0,       movingY, invertX, 0,       invertC);

  // Draw text in the middle of the screen
  // videoOut.setCursor(25, 80);
  // videoOut.setTextColor(invertC);
  // videoOut.setTextSize(2);
  // videoOut.setTextWrap(false);
  // videoOut.print("Adafruit GFX API");
  // videoOut.setCursor(110, 120);
  // videoOut.setTextColor(0xFF);
  // videoOut.print("on");
  // videoOut.setCursor(30, 160);
  // videoOut.setTextColor(cycle);
  // videoOut.print("ESP_8_BIT video");

  // videoOut.setCursor(100, 190);
  // videoOut.setTextColor(0xFF);
  // videoOut.print(analogRead(34));

  // int audioValue = analogRead(34);

  videoOut.setCursor(20, 20);
  videoOut.setTextSize(2);
  videoOut.setTextWrap(false);
  videoOut.setTextColor(0xFF);
  videoOut.print("audioValue");
  
  // // shift
  // for (int i = 255 - 1; i > 0; i--) {
  //   buffer[i] = buffer[i-1];
  // }
  // // push
  // buffer[0] = audioValue;


  // // draw
  for (int i = 0; i < 255; i++) {
    videoOut.drawPixel(255-i, (int16_t) (buffer[i] * 240 / 4096), 0x0C);
    // gain x2 also centered
    videoOut.drawPixel(255-i, (int16_t) (120 + (buffer[i] - 2048) * 240 * 2 / 4096), 0x0C);
  }
  
  counter = 0;
  }
  */


  // sample audio sig
  for (int i = 0; i < samples; i++) {
  // for (int i = 0; i < samples / 2; i++) {
    unsigned long now = micros();
    while (micros() - now < samplingPeriodUs);  // wait for interval

    int audioSignal = analogRead(34);
    vReal[i] = (double)(audioSignal - 1830);  // seems to rest around 1830.....
    vImag[i] = 0;
  }


  // compute
  FFT.windowing(FFTWindow::Hamming, FFTDirection::Forward); //optional?
  FFT.compute(FFTDirection::Forward);
  FFT.complexToMagnitude(); // compute magnitudes


  // visualize
  videoOut.waitForFrame();  // wait incase this is too fast
  videoOut.fillScreen(0);
  videoOut.setCursor(16, 16);
  videoOut.setTextSize(1);
  videoOut.print("audio_signal");

  for (int i = 1; i < samples / 2; i++) {
    // int magnitude = (int)vReal[i];
    int magnitude = max((int) vReal[i], (int) (smooth[i] * 0.65));

    smooth[i] = magnitude;

    // scale height (240)
    int height = constrain(map(magnitude, 0, 5000, 0, 200), 0, 200);

    // videoOut.drawFastVLine(i, 240-height, height, 0xFF);

    videoOut.drawFastVLine(i * ((int) 256 / samples * 2),     240 - height, height, 0xFF);
    videoOut.drawFastVLine(i * ((int) 256 / samples * 2) + 1, 240 - height, height, 0xFF);
  }
}
