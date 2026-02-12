#ifndef CRT_EFFECTS_H
#define CRT_EFFECTS_H

#include <ESP_8_BIT_GFX.h>

class CRTEffects {
private:
    ESP_8_BIT_GFX* _video;
    int _samples;
    double* _smooth;
    uint8_t _color;
    double* _vReal;
    uint32_t _frameCount = 0;

    // Helper: Gets average magnitude of a range of bins
    float getRangeAvg(int startBin, int endBin) {
        float sum = 0;
        for (int i = startBin; i <= endBin; i++) sum += _vReal[i];
        return sum / (endBin - startBin + 1);
    }
    // Smoothing logic: handles "rise" and "fall" of bars
    // int getSmoothedMag(int index, double currentVal) {
    //     if (index >= _samples / 2) return 0;
        
    //     // Use your original coefficients: 0.4 for rise, 0.8 for fall
    //     // This ensures spikes are fast but decay is smooth
    //     int mag = max((int)(currentVal * 0.4 + _smooth[index] * 0.6),
    //                   (int)(currentVal * 0.2 + _smooth[index] * 0.8));
        
    //     _smooth[index] = mag;
    //     return mag;
    // }
    int getSmoothedMag(int index, double currentVal, float rise = 0.5, float fall = 0.8) {
        if (index >= _samples) return 0;
        _smooth[index] = max((int)(currentVal * rise + _smooth[index] * (1.0 - rise)),
                             (int)(currentVal * (1.0 - fall) + _smooth[index] * fall));
        return (int)_smooth[index];
    }

public:
    CRTEffects(ESP_8_BIT_GFX* video, int samples) {
        _video = video;
        _samples = samples;
        _smooth = new double[samples / 2];
        // FIX: Ensure the array is empty at the start to prevent "The Flash"
        for (int i = 0; i < samples / 2; i++) _smooth[i] = 0;
    }

    void updateData(double* vReal, uint8_t color) {
        _vReal = vReal;
        _color = color;
    }

    // Effect 1: Bars from the bottom
    void colouredBarsBottom() {
        int barWidth = 256 / (_samples / 2);
        for (int i = 1; i < _samples / 2; i++) {
            int magnitude = getSmoothedMag(i, _vReal[i]);
            // Adjust 5000 to 2000 if your signal is too quiet
            int height = constrain(map(magnitude, 0, 5000, 0, 200), 0, 200);
            
            int x = i * barWidth;
            for(int w=0; w < barWidth-1; w++) {
                _video->drawFastVLine(x + w, 240 - height, height, _color);
            }
        }
    }

    // Effect 2: Bars from the center
    void colouredBarsCenter() {
        int barWidth = 256 / (_samples / 2);
        for (int i = 1; i < _samples / 2; i++) {
            int magnitude = getSmoothedMag(i, _vReal[i]);
            int height = constrain(map(magnitude, 0, 5000, 0, 200), 0, 200);
            
            int x = i * barWidth;
            for(int w=0; w < barWidth-1; w++) {
                _video->drawFastVLine(x + w, 120 - (height / 2), height, _color);
            }
        }
    }

    // Effect 3: Logarithmic Frequency Bars (The "Pro" Look)
    void logColouredBarsTick(float exponent = 2.3) {
        const int numBins = 32;
        int binWidth = 256 / numBins;

        for (int i = 0; i < numBins; i++) {
            float startF = pow(i / (float)numBins, exponent) * (_samples / 2);
            float endF   = pow((i + 1) / (float)numBins, exponent) * (_samples / 2);

            int startIdx = constrain((int)startF, 1, (_samples / 2) - 1);
            int endIdx   = constrain((int)endF, startIdx + 1, (_samples / 2));

            float sum = 0;
            for (int j = startIdx; j < endIdx; j++) sum += _vReal[j];
            float avg = sum / (endIdx - startIdx);

            int magnitude = getSmoothedMag(i, avg);
            int height = constrain(map(magnitude, 0, 5000, 0, 200), 0, 200);
            
            _video->fillRect(i * binWidth, 120 - (height / 2), binWidth - 2, height, _color);
        }
    }

    // Effect 4: Simple White Oscilloscope Line
    void whiteLineTick() {
        int lastX = 0;
        int lastY = 120;
        int step = 256 / (_samples / 2);
        
        for (int i = 1; i < _samples / 2; i++) {
            int magnitude = getSmoothedMag(i, _vReal[i]);
            int height = constrain(map(magnitude, 0, 5000, 0, 100), 0, 100);
            int currentX = i * step;
            int currentY = 120 - height;
            _video->drawLine(lastX, lastY, currentX, currentY, 0xFF); 
            lastX = currentX;
            lastY = currentY;
        }
    }

    // Effect 5: Windows XP "Pulsing" Style
    void pulsingOrbs() {
        // Sample Low, Mid, High
        int bass = getSmoothedMag(0, _vReal[2]);
        int mid  = getSmoothedMag(1, _vReal[_samples/8]);
        int high = getSmoothedMag(2, _vReal[_samples/4]);

        _video->drawCircle(128, 120, map(bass, 0, 5000, 5, 80), _color);
        _video->drawCircle(128, 120, map(mid, 0, 5000, 2, 50), _color | 0x1C);
        _video->drawCircle(128, 120, map(high, 0, 5000, 1, 30), 0xFF);
    }


    void nebulaTunnel() {
        int bass = getSmoothedMag(0, getRangeAvg(2, 5));
        int speed = map(bass, 0, 5000, 2, 15);
        
        for (int i = 0; i < 20; i++) {
            // Use framecount to create a deterministic "path" for stars
            float angle = (i * 18) * 0.01745; 
            int dist = (_frameCount * speed + (i * 20)) % 150;
            int x = 128 + cos(angle) * dist;
            int y = 120 + sin(angle) * dist;
            if (x > 0 && x < 255 && y > 0 && y < 240)
                _video->drawPixel(x, y, (dist > 100) ? 0xFF : _color);
        }
    }

    // --- EFFECT: SONIC RINGS (The "Crazy" Orb) ---
    // 10 concentric rings. Bass handles the inner, Treble handles the outer.
    void sonicRings() {
        for (int i = 0; i < 10; i++) {
            // We sample across the whole spectrum for different rings
            float raw = getRangeAvg(i * 3, (i + 1) * 3);
            int val = getSmoothedMag(i + 20, raw); 
            int radius = (i * 12) + map(val, 0, 4000, 0, 25);
            
            // Alternate colors between your selected hue and white
            uint8_t c = (i % 2 == 0) ? _color : (_color | 0x1C);
            _video->drawCircle(128, 120, radius, c);
            if (val > 3000) _video->drawCircle(128, 120, radius + 1, 0xFF);
        }
    }

    // --- EFFECT: THE WAVELINE (Classic XP Wave) ---
    // A flowing line that deforms based on frequency
    void oceanWave() {
        int lastX = 0, lastY = 120;
        for (int i = 1; i < 32; i++) {
            float raw = getRangeAvg(i, i);
            int val = getSmoothedMag(i + 40, raw);
            int x = i * 8;
            // Use sin waves to make it "floaty" even when quiet
            int offset = sin((_frameCount + i * 5) * 0.1) * 10;
            int y = 120 - map(val, 0, 5000, 0, 80) + offset;
            
            _video->drawLine(lastX, lastY, x, y, _color);
            _video->drawLine(lastX, lastY + 1, x, y + 1, 0xFF); // Highlight
            lastX = x; lastY = y;
        }
    }

    // --- EFFECT: BASS TONGUE (Screen Shake) ---
    // The whole "floor" of the screen bounces to the kick drum
    void bassTongue() {
        int bass = getSmoothedMag(1, getRangeAvg(2, 4));
        int h = map(bass, 0, 6000, 0, 100);
        for(int i=0; i<h; i+=4) {
            uint8_t shade = (i < 40) ? 0x03 : _color; // Dimmer at bottom
            _video->drawFastHLine(128 - (i*1.2), 240 - i, (i*2.4), shade);
        }
    }
    
    // --- EFFECT: RADAR SCAN (Geometric Pulse) ---
    void radarScan() {
        int mid = getSmoothedMag(2, getRangeAvg(10, 20));
        float angle = (_frameCount % 360) * 0.01745;
        int len = 40 + map(mid, 0, 3000, 0, 60);
        int x = 128 + cos(angle) * len;
        int y = 120 + sin(angle) * len;
        _video->drawLine(128, 120, x, y, 0xFF);
        _video->drawRect(128 - len/2, 120 - len/2, len, len, _color);
    }
};

#endif