/** 
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

// #define HELIX_LOGGING_ACTIVE false

#include "AudioTools.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "AudioTools/Disk/AudioSourceSDFAT.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
// #include "SD.h"

#define playPauseBtn 25
#define nextBtn 26
#define prevBtn 27
#define potPin 36

const char *startFilePath="/";
const char* ext="mp3";
AudioSourceSDFAT source(startFilePath, ext); // , PIN_AUDIO_KIT_SD_CARD_CS);
A2DPStream out;
MP3DecoderHelix decoder;
AudioPlayer player(source, out, decoder);


const unsigned long debounceDelay = 200;
unsigned long lastPlayPause = 0;
unsigned long lastNext = 0;
unsigned long lastPrev = 0;
float currentVolume = 0.5;
float lastVolume = 0.5;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Program started ===");
  // if(!SD.begin(5)) {
  //   Serial.println("SD init failed!");
  // } else {
  //   Serial.println("SD init OK");
  // }
  pinMode(playPauseBtn, INPUT_PULLUP);
  pinMode(nextBtn, INPUT_PULLUP);
  pinMode(prevBtn, INPUT_PULLUP);
  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  // Serial.println("PLAYING IAN");
  // setup player
  // Setting up SPI if necessary with the right SD pins by calling 
  // SPI.begin(PIN_AUDIO_KIT_SD_CARD_CLK, PIN_AUDIO_KIT_SD_CARD_MISO, PIN_AUDIO_KIT_SD_CARD_MOSI, PIN_AUDIO_KIT_SD_CARD_CS);
  pinMode(potPin, INPUT);
  player.setVolume(currentVolume);
  player.begin();

  // setup output - We send the test signal via A2DP - so we conect to a Bluetooth Speaker
  auto cfg = out.defaultConfig(TX_MODE);
  cfg.silence_on_nodata = true; // prevent disconnect when there is no audio data
  cfg.name = "Jabra Elite Active 65t";  // set the device here. Otherwise the first available device is used for output
  // cfg.auto_reconnect = true;  // if this is use we just quickly connect to the last device ignoring cfg.name
  out.begin(cfg);


}

void loop() {
  // Play/Pause button
  if (digitalRead(playPauseBtn) == LOW && (millis() - lastPlayPause > debounceDelay)) {
    lastPlayPause = millis();
    if (player.isActive()) {
      player.stop();
    } else {
      player.begin();
    }
  }

  // next song button
  if (digitalRead(nextBtn) == LOW && (millis() - lastNext > debounceDelay)) {
    lastNext = millis();
    player.next();
  }

    // prev song button
  if (digitalRead(prevBtn) == LOW && (millis() - lastPrev > debounceDelay)) {
    lastPrev   = millis();
    player.previous();
  }

  int potValue = analogRead(potPin);  // 0 - 4095
  float newVolume = potValue / 4095.0;

  if (abs(newVolume - lastVolume) > 0.02) {
    player.setVolume(newVolume);
    lastVolume = newVolume;
  }

  player.copy();
}
