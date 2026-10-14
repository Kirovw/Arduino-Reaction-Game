# Arduino Reaction Game

A reaction time game for Arduino Uno: an LED lights up at a random moment, you press the button as fast as you can, and your time is logged with a timestamp to a CSV file on an SD card. Use it to see how your reaction time changes throughout the day.

<!-- ![Project photo](images/project.jpg) -->

## How it works

Press the button to start a round. After 2-5 seconds the LED turns on and you press the button again. The result is saved to `REACT.CSV`.

- Reaction under 250 ms: the servo nods
- Pressing before the LED lights up (false start): `false_start` is logged and the servo turns away
- No press within 3 seconds: `timeout` is logged

```
date,time,reaction_ms
2026-10-15,09:12:41,243
2026-10-15,09:13:02,false_start
```

## Components

Arduino Uno, data logger shield (RTC + SD), CR1220 battery, SD card (FAT32), LED, 220 Ω resistor, push button, SG90 servo (optional), breadboard, jumper wires.

## Wiring

| Arduino | Connects to |
|---|---|
| D2 | button (other side to GND) |
| D6 | LED anode (cathode through 220 Ω to GND) |
| D9 | servo signal |
| 5V / GND | servo power |

The button has 4 legs that are internally connected in pairs. Connect your wires to diagonally opposite legs.

## Getting started

1. Install the **RTClib** library (Adafruit) from the Library Manager.
2. Open `reaction_game_servo/reaction_game_servo.ino` and set your RTC type in the code (`RTC_DS1307`, `RTC_DS3231` or `RTC_PCF8523`).
3. Upload the sketch to the Arduino Uno and open the Serial Monitor (9600 baud).

To analyze the data, open `REACT.CSV` in Excel or Google Sheets and chart the time of day on the X axis against reaction time on the Y axis.

## Known issues

Small servos can draw more current than a USB port provides. If the servo hums but does not turn, add a 100-470 µF capacitor between 5V and GND or power it separately.

## License

MIT
