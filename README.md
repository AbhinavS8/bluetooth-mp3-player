# bluetooth-mp3-player

An MP3 player project that uses an ESP32 to read MP3 files from an SD card and stream audio to Bluetooth earbuds/headphones. This repository contains separate Arduino sketches and helper headers for SD card interfacing, Bluetooth streaming, and basic playback controls.

> Status: Work in progress

## Features
- Streams audio from an SD card using an ESP32.
- Sends audio over Bluetooth so you can use Bluetooth earbuds/headphones as the output device.
- Basic playback controls (play/pause/skip) and volume

## Hardware required
- ESP32
- SD card and SD card module
- Bluetooth earbuds/headphones (A2DP sink to receive streamed audio).
- Buttons + potentiometer


## Installation and usage
1. Format your microSD card as FAT32 and copy your MP3 files to the root of the card. Use simple ASCII filenames (no spaces or special characters) for best compatibility.
2. Wire the SD card module to the ESP32:
   - Connect MOSI, MISO, SCK and CS to the corresponding SPI pins on your ESP32 board.
   - Provide power (3.3V) and GND to the SD module.
   - If the sketches use SD_MMC, wiring may differ — check the sd_card_interfacing.ino implementation for required pins.
3. Place all project files (the .ino files and skipAudio.h) in a single folder named e.g. bluetooth-mp3-player and open bluetooth.ino in the Arduino IDE.
4. Select your ESP32 board and correct COM port in the Arduino IDE.
5. Install any missing libraries that appear in the #include statements.
6. Upload the sketch to the ESP32.
7. Open your earbuds/headphones Bluetooth settings and pair them with the ESP32 device (the device name may be set in the sketch).
8. Once paired, playback should stream MP3 content from the SD card to the Bluetooth device. Use any implemented hardware or software controls for play/pause/skip as described in the code.

<img width="1280" height="1098" alt="image" src="https://github.com/user-attachments/assets/9366796c-d566-4f24-ac90-94e2ee3871e1" />
