/**
 * @file player-sdfat-a2dp.ino
 * @brief see https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-player/player-sdfat-a2dp/README.md
 * 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

//remove this, to find issues regarding mp3 decoding
// #define HELIX_LOGGING_ACTIVE false

#include "AudioTools.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "AudioTools/Disk/AudioSourceSDFAT.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"

const char *startFilePath="/";
const char* ext="mp3";
AudioSourceSDFAT source(startFilePath, ext); // , PIN_AUDIO_KIT_SD_CARD_CS);
A2DPStream out;
MP3DecoderHelix decoder;
AudioPlayer player(source, out, decoder);

const int playPauseBtn = 26;
const int nextBtn = 25;
const int prevBtn = 27;

// ===== Debounce Timing =====
const unsigned long debounceDelay = 200;
unsigned long lastPlayPause = 0;
unsigned long lastNext = 0;
unsigned long lastPrev = 0;
bool paused = false;

// volume
const int potPin = 35;
float lastVolume = 0.8;

void setup() {
  Serial.begin(115200);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Warning);

  pinMode(playPauseBtn, INPUT_PULLUP);
  pinMode(nextBtn, INPUT_PULLUP);
  pinMode(prevBtn, INPUT_PULLUP);
  pinMode(potPin, INPUT);
  // setup player
  // Setting up SPI if necessary with the right SD pins by calling 
  // SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);
  player.setVolume(0.8);
  player.begin();

  // setup output - We send the test signal via A2DP - so we conect to a Bluetooth Speaker
  auto cfg = out.defaultConfig(TX_MODE);
  cfg.silence_on_nodata = true; // prevent disconnect when there is no audio data
  cfg.name = "";  // set the device here. Otherwise the first available device is used for output
  //cfg.auto_reconnect = true;  // if this is use we just quickly connect to the last device ignoring cfg.name
  out.begin(cfg);

  

}

void loop() {


  // --- Volume Control ---
  int potValue = analogRead(potPin);

  // Convert 0-4095 -> 0.0-1.0
  float volume = potValue / 4095.0f;

  // Prevent constant tiny updates
  if (abs(volume - lastVolume) > 0.02f) {

      player.setVolume(volume);

      Serial.print("Volume: ");
      Serial.println(volume);

      lastVolume = volume;
  }
  // --- Play / Pause ---
  if (digitalRead(playPauseBtn) == LOW &&
      millis() - lastPlayPause > debounceDelay) {

    lastPlayPause = millis();

    paused = !paused;

    if (paused) {
      Serial.println("Paused");
      player.stop();
    } else {
      Serial.println("Playing");
      player.play();
    }
  }

  // --- Next ---
  if (digitalRead(nextBtn) == LOW &&
      millis() - lastNext > debounceDelay) {

    lastNext = millis();

    Serial.println("Next");
    player.next();

    paused = false;
  }

  // --- Previous ---
  if (digitalRead(prevBtn) == LOW &&
      millis() - lastPrev > debounceDelay) {

    lastPrev = millis();

    Serial.println("Previous");
    player.previous();

    paused = false;
  }

  player.copy();
}
