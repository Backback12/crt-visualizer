# crt-visualizer

Visualize audio input through composite video output
Built using ESP32 (LOLIN D32)


## Libraries
### ESP_8_BIT_composite
https://github.com/Roger-random/ESP_8_BIT_composite/

### arduinoFFT
https://github.com/kosme/arduinoFFT/wiki




# Electrical Schematic
ASCII box characters for copy and pasting lol
```
┌┬┐
├┼┤
└┴┘
─│
●o
```

### Input Audio DC Bias Circuit
```
[L Audio]             [3.3V]
     │                  │
     █ 10kΩ             █ 100kΩ
     ├────────┬──┤├─────┼────────● GPIO 34
     █ 10kΩ   │ 1µF     █ 100kΩ
     │        │         │
[R Audio]     │       [GND]
              │
      [Mono Audio Out]
```

### Potentiometer Schematic

```
        1kΩ
GND ──/\/\/\/\── 3.3V
          ^
          │
       GPIO 33
```
(Currently is 1kΩ Potentiometer but plan to upgrade to 10kΩ to reduce power consumption)

### Push Button Schematic
```

       ─┴─
GND ──o   o── GPIO 32
```
