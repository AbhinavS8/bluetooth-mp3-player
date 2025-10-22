#ifndef SKIP_BYTES_STREAM_H
#define SKIP_BYTES_STREAM_H

#include "AudioTools/CoreAudio/AudioStreams.h"
#include <stdint.h>
#include <string.h>

/**
 * SkipBytesStream
 *
 * - Lightweight AudioStream wrapper that discards the first N bytes read from an
 *   underlying Stream and then forwards subsequent bytes unchanged.
 * - Does NOT use dynamic_cast or RTTI.
 * - Works solely via the Stream interface (readBytes/write/available).
 * - If you have an AudioStream or know the audio format, call setAudioInfo(...) to
 *   provide AudioInfo to the wrapper so downstream components can get correct info.
 *
 * Note:
 * - If your underlying file/stream type supports seek and you have access to the
 *   concrete API (File or SdFat types), you can optimize skipping by seeking on that
 *   concrete file handle before calling setStream(...). That avoids reading and discarding
 *   many bytes.
 */
class SkipBytesStream : public audio_tools::AudioStream {
public:
  SkipBytesStream() : p_stream(nullptr), skipRemaining(0), active(false) {}

  // Attach an underlying Stream (by reference)
  // This accepts any Stream and stores a pointer; no RTTI used.
  void setStream(Stream &s) {
    p_stream = &s;
  }

  // Set number of bytes to skip on the next readBytes call(s)
  void setSkip(size_t n) {
    skipRemaining = n;
  }

  // Optional: provide AudioInfo when you know it (no runtime cast necessary)
  void setAudioInfoExtern(const audio_tools::AudioInfo &info) {
    audio_tools::AudioStream::setAudioInfo(info);
  }

  bool begin() override {
    active = true;
    // Do NOT call begin() on p_stream here because p_stream is a generic Stream*
    // and may not implement begin(); callers should initialize the underlying
    // stream before attaching it.
    return true;
  }

  void end() override {
    active = false;
  }

  int available() override {
    if (!active || p_stream == nullptr) return 0;
    int a = p_stream->available();
    Serial.println(a);
    if (a <= 0) return a;
    if (skipRemaining == 0) return a;
    // approximate available after skipping (can't know exact without discarding or seeking)
    if ((size_t)a <= skipRemaining) return 0;
    return a - (int)skipRemaining;
  }

  // readBytes: discard skipRemaining bytes first (by reading into small temp buffer),
  // then provide requested bytes to caller.
  size_t readBytes(uint8_t *data, size_t len) override {
    if (!active || p_stream == nullptr || len == 0) return 0;
    Serial.println("skipaudio reading..");
    // discard skipRemaining by reading into a small tmp buffer
    while (skipRemaining > 0) {
      size_t toDiscard = skipRemaining;
      // limit chunk size to avoid very large stack buffers
      if (toDiscard > 512) toDiscard = 512;
      uint8_t tmp[TMP_MAX];
      size_t r = p_stream->readBytes(tmp, toDiscard);
      if (r == 0) {
        // no data available from underlying stream - nothing to return now
        return 0;
      }
      skipRemaining -= r;
      // continue discarding until skipRemaining = 0
    }

    // now read the requested data
    Serial.println("reached end");
    return p_stream->readBytes(data, len);
  }

  // forward single-byte write to underlying stream if supported
  size_t write(const uint8_t *data, size_t len) override {
    if (p_stream == nullptr) return 0;
    return p_stream->write(data, len);
  }

  // single char write helper
  size_t write(uint8_t ch) override {
    if (p_stream == nullptr) return 0;
    return p_stream->write(ch);
  }


protected:
  Stream *p_stream;
  size_t skipRemaining;
  bool active;
};

#endif // SKIP_BYTES_STREAM_H