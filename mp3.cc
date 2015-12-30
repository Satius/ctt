#include "util.h"
#include "mp3.h"
#include <algorithm>
#include <fstream>
#include <vector>
#include <lame/lame.h>

namespace {

const std::size_t kBufferSize = 128 * 1024;

using LamePtr = std::unique_ptr<lame_global_flags, decltype(&lame_close)>;

struct OutputMp3StreamImpl : public mp3::IOutputMp3Stream {
 private:
  std::vector<uint8_t> buffer_;
  std::ofstream dst_;
  mp3::Info info_;
  bool failure_;
  LamePtr lame_;

  inline int EncodeSamples(const uint8_t* src_buffer, int num_samples, uint8_t* dst_buffer, int dst_size) {
    std::unique_ptr<int16_t[]> resampled;
    int16_t* src = nullptr;
    switch (info_.bits_per_sample) {
      case 8:
        resampled.reset(new int16_t[num_samples * info_.num_channels]);
        src = resampled.get();
        for (int i = 0; i < num_samples * info_.num_channels; ++i)
          src[i] = (src_buffer[i] - 128) << 8;
        break;
      case 16:
        src = const_cast<int16_t*>(reinterpret_cast<const int16_t*>(src_buffer));
        break;
      default: throw std::runtime_error("Invalid internal state");
    }
    switch (info_.num_channels) {
      case 1: return lame_encode_buffer(lame_.get(), src, nullptr, num_samples, dst_buffer, dst_size);
      case 2: return lame_encode_buffer_interleaved(lame_.get(), src, num_samples, dst_buffer, dst_size);
      default: throw std::runtime_error("Invalid internal state");
    }
  }

 public:
  OutputMp3StreamImpl(const std::string& fname) :
      buffer_(kBufferSize),
      dst_(fname, std::ios::binary),
      failure_(false),
      lame_(lame_init(), lame_close) {
    if (!dst_) {
      const auto& message = util::Join(util::kSpaceString, "Can not open file", fname, "for writing");
      util::ThrowSystemError(message);
    }
    if (!lame_)
      throw std::runtime_error("Lame failed to initialize");
  }

  void SetInfo(const mp3::Info& info) override {
    if (info.num_channels != 1 && info.num_channels != 2) {
      const auto& message = util::Join(util::kSpaceString, "Unexpected number of channels", info.num_channels);
      throw std::runtime_error(message);
    }
    if (info.bits_per_sample != 8 && info.bits_per_sample != 16) {
      const auto& message = util::Join(util::kSpaceString, "Unexpected bits per sample", info.bits_per_sample);
      throw std::runtime_error(message);
    }
    info_ = info;
    if (lame_set_num_channels(lame_.get(), info_.num_channels))
      throw std::runtime_error("Failed to set number of channels");
    if (lame_set_mode(lame_.get(), info_.num_channels == 1 ? MONO : STEREO))
      throw std::runtime_error("Failed to set lame mode");
    if (lame_set_in_samplerate(lame_.get(), info_.sample_rate))
      throw std::runtime_error("Failed to set lame sample rate");
    if (lame_set_quality(lame_.get(), info_.quality))
      throw std::runtime_error("Failed to set lame quality");
    if (lame_init_params(lame_.get()))
      throw std::runtime_error("Failed to apply lame configuration");
  }

  std::size_t Write(const uint8_t* buffer, std::size_t size) override {
    try {
      const auto item_size = (info_.bits_per_sample * info_.num_channels) >> 3;
      for (decltype(size) iter = 0; iter < size; iter += buffer_.size()) {
        auto in_size = std::min(size, iter + buffer_.size()) - iter;
        if (in_size % item_size) throw std::runtime_error("Misaligned data block size");
        auto out_size = EncodeSamples(buffer + iter, in_size / item_size, buffer_.data(), buffer_.size());
        if (out_size < 0) throw std::runtime_error("Lame returned error while encoding");
        dst_.write(reinterpret_cast<const char*>(buffer_.data()), out_size);
        if (!dst_) util::ThrowSystemError("File writing failed");
      }
      return size;
    } catch (const std::exception&) {
      failure_ = true;
      throw;
    }
  }

  ~OutputMp3StreamImpl() {
    if (failure_) return;
    auto out_size = lame_encode_flush(lame_.get(), buffer_.data(), buffer_.size());
    if (out_size > 0) dst_.write(reinterpret_cast<char*>(buffer_.data()), out_size);
  }
};

}  // namespace

namespace mp3 {

OutputMp3StreamPtr WriteMp3File(const std::string& fname) {
  return OutputMp3StreamPtr(new OutputMp3StreamImpl(fname));
}

}  // namespace wav
