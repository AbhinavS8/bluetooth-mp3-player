#include <Arduino.h>
#include <algorithm> // for std::min
#include "AudioTools.h"
#include "AudioTools/AudioLibs/A2DPStream.h"
#include "AudioTools/Disk/AudioSourceSDFAT.h"
#include "AudioTools/AudioCodecs/CodecMP3Helix.h"
#include "skipAudio.h"

#define playPauseBtn 25
#define nextBtn 26
#define prevBtn 27
#define potPin 36

const char *startFilePath = "/";
const char *ext = "mp3";

// === Audio chain ===
// Source -> Decoder -> A2DP Output
AudioSourceSDFAT source(startFilePath, ext);
A2DPStream out;
MP3DecoderHelix decoder;
AudioPlayer player(source, out, decoder);

// --- Byte counting state and callback ----------------------------------
static size_t bytesCopiedSinceStart = 0; 
static bool copyingCallbackRegistered = false;

void bytesCopiedCallback(void *obj, void *buffer, size_t len) {
  bytesCopiedSinceStart += len;  // track how many bytes we’ve sent into decoder
}


static SkipBytesStream skipStream;

// --- Pause/Resume bookkeeping --------------------------------------------
unsigned long currentFileIndex = 0;
size_t savedByteOffset = 0;
bool isPaused = false;

// UI debounce & volume state
const unsigned long debounceDelay = 200;
unsigned long lastPlayPause = 0;
unsigned long lastNext = 0;
unsigned long lastPrev = 0;
float currentVolume = 0.5;
float lastVolume = 0.5;

void setup() {
  Serial.begin(115200);
  Serial.println("=== Program started ===");

  pinMode(playPauseBtn, INPUT_PULLUP);
  pinMode(nextBtn, INPUT_PULLUP);
  pinMode(prevBtn, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Info);
  player.setVolume(currentVolume);

  // Register StreamCopy byte-counting callback once
  if (!copyingCallbackRegistered) {
    player.getStreamCopy().setCallbackOnWrite(bytesCopiedCallback, nullptr);
    copyingCallbackRegistered = true;
  }

  // Start player (selects first stream by begin())
  if (!player.begin()) {
    
    Serial.println("Player.begin() failed!");
  }

  // Configure A2DP output
  auto cfg = out.defaultConfig(TX_MODE);
  cfg.silence_on_nodata = true;
  cfg.name = "";
  // cfg.auto_reconnect = true;
  out.begin(cfg);

  // Initialize counters
  bytesCopiedSinceStart = 0;
  savedByteOffset = 0;
  isPaused = false;
}

void loop() {
  // --- Play/Pause Button ---
  if (digitalRead(playPauseBtn) == LOW && (millis() - lastPlayPause > debounceDelay)) {
    lastPlayPause = millis();

    if (player.isActive()) {
      Serial.println("Pausing...");
      player.stop();
      player.setAutoNext(false);
      savedByteOffset = bytesCopiedSinceStart;
      isPaused = true;
      Serial.printf("Saved byte offset: %u\n", (unsigned)savedByteOffset);
      decoder.end();
    } else {
      Serial.println("Resuming...");
      int idx = source.index();
      if (idx < 0)
        idx = 0;

      Serial.printf("Resuming index: %d\n", idx);
      Stream *fileStream = source.selectStream(idx);
      if (!fileStream) {
        Serial.println("Could not open file to resume!");
      } else {
        skipStream.setStream(*fileStream);
        skipStream.setSkip(savedByteOffset);

        decoder.begin();
        player.setStream(&skipStream);
        player.play();
        player.setAutoNext(true);

        isPaused = false;
      }
    }
  }

  // --- Next Song ---
  if (digitalRead(nextBtn) == LOW && (millis() - lastNext > debounceDelay)) {
    lastNext = millis();
    bytesCopiedSinceStart = 0;
    savedByteOffset = 0;
    player.next();
  }

  // --- Previous Song ---
  if (digitalRead(prevBtn) == LOW && (millis() - lastPrev > debounceDelay)) {
    lastPrev = millis();
    bytesCopiedSinceStart = 0;
    savedByteOffset = 0;
    player.previous();
  }

  // --- Volume Potentiometer ---
  int potValue = analogRead(potPin);
  float newVolume = potValue / 4095.0f;
  if (fabs(newVolume - lastVolume) > 0.05f) {
    player.setVolume(newVolume);
    lastVolume = newVolume;
  }

  // --- Copy step (core audio loop) ---
  player.copy();
}
