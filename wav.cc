#include "util.h"
#include "wav.h"
#include <fstream>

namespace {

// Endianess and data alignment are not handled here!

#pragma pack(push, 1)
struct WavHeader {
  uint32_t riff_header;
  uint32_t riff_size;
  uint32_t wave_label;
  uint32_t fmt_header;
  uint32_t fmt_size;
  uint16_t audio_format;
  uint16_t num_channels;
  uint32_t sample_rate;
  uint32_t byte_rate;
  uint16_t block_align;
  uint16_t bits_per_sample;
};
#pragma pack(pop)

const std::size_t kFmtOffset = 5 * sizeof(uint32_t);
const uint16_t kPcmFormat = 1;
const uint32_t kDataHeader = 0x61746164;  // data
const uint32_t kFmtHeader = 0x20746d66;  // fmt_
const uint32_t kRiffHeader = 0x46464952;  // RIFF
const uint32_t kWaveLabel = 0x45564157;  // WAVE

wav::Info ReadWavInfo(std::istream& src) {
  WavHeader header;
  src.read(reinterpret_cast<char*>(&header), sizeof(header));
  if (!src || src.gcount() != sizeof(header))
    util::ThrowSystemError("File is invalid");
  if (header.riff_header != kRiffHeader)
    throw std::runtime_error("File is not a RIFF file");
  if (header.wave_label != kWaveLabel)
    throw std::runtime_error("File is not a WAVE file");
  if (header.fmt_header != kFmtHeader)
    throw std::runtime_error("WAVE file is damaged");
  if (header.audio_format != kPcmFormat) {
    const auto& message = util::Join("Audio is not in PCM format", header.audio_format);
    throw std::runtime_error(message);
  }
  auto next_chunk = kFmtOffset + header.fmt_size;
  src.seekg(next_chunk - sizeof(header), std::ios_base::cur);
  if (!src) util::ThrowSystemError("Unexpected error while reading header");
  return {header.num_channels, static_cast<int>(header.sample_rate), header.bits_per_sample};
}

bool FindDataChunk(std::istream& src, std::size_t& chunk_size) {
  for (;;) {
    uint32_t chunk_header[2];
    src.read(reinterpret_cast<char*>(chunk_header), sizeof(chunk_header));
    if (src.eof()) return false;
    if (!src || src.gcount() != sizeof(chunk_header))
      util::ThrowSystemError("Failed to read chunk header");
    if (chunk_header[0] == kDataHeader) {
      chunk_size = chunk_header[1];
      return true;
    }
    src.seekg(chunk_header[1], std::ios_base::cur);
  }
}

class InputWavStreamImpl : public wav::IInputWavStream {
 private:
  std::size_t chunk_size_;
  std::size_t chunk_position_;
  std::ifstream src_;
  wav::Info info_;

 public:
  InputWavStreamImpl(const std::string& fname) :
      chunk_size_(0),
      chunk_position_(0),
      src_(fname),
      info_(ReadWavInfo(src_)) {
    if (!src_) {
      const auto& message = util::Join("Can not open file", fname, "for reading");
      util::ThrowSystemError(message);
    }
  }

  wav::Info GetInfo() const override {
    return info_;
  }

  std::size_t Read(uint8_t* buffer, std::size_t size) override {
    if (!chunk_size_ && !FindDataChunk(src_, chunk_size_)) return 0;
    auto op_size = std::min(size, chunk_size_ - chunk_position_);
    src_.read(reinterpret_cast<char*>(buffer), op_size);
    if (!src_ || static_cast<std::size_t>(src_.gcount()) != op_size)
      util::ThrowSystemError("File reading failed");
    if ((chunk_position_ += op_size) == chunk_size_) {
      chunk_size_ = 0;
      chunk_position_ = 0;
    }
    return op_size;
  }
};

}  // namespace

namespace wav {

InputWavStreamPtr ReadWavFile(const std::string& fname) {
  return std::make_unique<InputWavStreamImpl>(fname);
}

}  // namespace wav
