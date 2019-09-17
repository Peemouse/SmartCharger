# SmartCharger
Smart charger for universal Li-Ion battery pack charging

## Features :
- 1 to 12S Li-Ion charging
- Storage mode charging (no discharging)
- User friendly menu
- Safety features for battery, charger ans power supply protection
- Charge current ramp up
- End of charge detection
- Web server for remote monitoring and control (using Wifi)
- FW update OTA (using Wifi)

## Wiring (component pin name -> ESPio 32 pin number)

### Power supply
- 5V hacked on the USB charger -> pin 5V
- 0V hacked on the USB charger -> pin GND

### OLED display (Adafruit monochrome OLED 1.3" 128x64)
- CLK -> pin 22
- Data -> pin 21
- RST -> pin 27
- GND -> GND
- Vin -> 5V bus

### Buttons
- Down button -> pin 13
- Enter button -> pin 14
- Up button -> pin 12
- Common pin -> GND

### DPS communication
- Tx (yellow wire) -> pin 16
- Rx (black wire) -> pin 17
- GND (green wire) -> GND
- 3.3V from DPS (red) -> **DO NOT CONNECT** (isolate it instead)

### Active Buzzer
- Signal -> pin 26
- Vdd -> 5V bus
- GND -> GND

### Currently in beta version
