# ESP32radio

A ESP32 based Radio. Take a look at my homepage for more details.
https://www.mortara.org/allgemein/esp32-radio/

# Components

The main components are :

- A Lolin S3 ESP32 as main controller
- A NE5532 base pre-amplifier module
- A 2 x 50W amplifier module to drive the main speakers
- A Si4735 for FM/AM reception
- A VS1053 to play internet radio streams
- A HD44780 LCD for clock and status information on the right side
- A MAX7219 8 x 32 LED matrix display for current station and frequency on the left
- A 8 channel magnet relais for input switching
- A 4 channel magnet relais for speaker switching
- 2 x PCF8574 io expander
- A DS3231 real time clock module
- A INA219 module to measure current power consumption
- A BMP180 temperature sensor to measure the temp on the pre-amplifiers bridge rectifiers
- A small fan to cool down the bridge rectifiers if they get too hot
- A 220V to 18V transformer as main power source
- Several LED, cables, switches etc

# Wiring

ESP  |   Device                 |         Lolin S3 Pin
-----|--------------------------|--------------------
1    |                           |
2    |   Si4735 RST               |      -> 47
3    |                            |
4    |  VS1053 DREQ               |     -> 15
5    |  VS1053 MP3_CS             |     -> 16
6    |                            |
7   |                            |
8   |                            |
9   |                            |
10   |                            |
11   |                            |
12   |                            |
13   |  Freq. display Data        |     -> 8
14   |  Freq. display Clock       |     -> 3
15   |  Freq. display CS          |     -> 46
16   |  VS1053 XDCS               |     -> 4
17   |  VS1053 SD_CS              |     -> 5
18   |  VS1053 CLOCK              |     -> 6
19   |  VS1053 MISO               |     -> 7
20   |                            |
21   |  I2C 1 SDA                 |     -> 42
22   |  I2C 1 SCL                 |     -> 41
23   |  VS1053 MOSI               |     -> 18
24   |                            |
25   |  DAC Signal meter          |     X
26   |  DAC Frequency meter       |     X
27   |  Speaker Relais            |     -> 21
28   |                            |
29   |                            |
30   |                            |
31   |                            |
32   |  I2C 2 SCL                 |     -> 39
33   |  I2C 2 SDA                 |     -> 40
34   |  Rotary encoder cw         |     -> 45
35   |  Rotary encoder ccw        |     -> 48
36   |                            |
37   |                            |
38   |                            |
39   |  Rotary encoder button     |     -> 14
40   |                            |

