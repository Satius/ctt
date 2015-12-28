#include "util.h"
#include "mp3.h"
#include <fstream>
#include <vector>
#include <lame/lame.h>

namespace {

const std::size_t kBufferSize = 128 * 1024;
const int kLameQuality = 5;

using LamePtr = std::unique_ptr<lame_global_flags, decltype(&lame_close)>;

struct OutputMp3StreamImpl : public mp3::IOutputMp3Stream {
 private:
  std::vector<uint8_t> buffer_;
  std::ofstream dst_;
  mp3::Info info_;
  bool failure_;
  LamePtr lame_;

 public:
  OutputMp3StreamImpl(const std::string& fname) :
      buffer_(kBufferSize),
      dst_(fname),
      failure_(false),
      lame_(lame_init(), lame_close) {
    if (!dst_) {
      const auto& message = util::Join("Can not open file", fname, "for writing");
      util::ThrowSystemError(message);
    }
    if (!lame_)
      throw std::runtime_error("Lame failed to initialize");
    if (lame_set_quality(lame_.get(), kLameQuality))
      throw std::runtime_error("Failed to set lame quality");
    if (lame_init_params(lame_.get()))
      throw std::runtime_error("Failed to apply lame configuration");
  }

  void SetInfo(const mp3::Info& info) override {
    info_ = info;
  }

  std::size_t Write(const uint8_t* buffer, std::size_t size) override {
    try {
      // TODO: Implement other formats
      for (decltype(size) iter = 0; iter < size; iter += buffer_.size()) {
        auto ptr = reinterpret_cast<const int16_t*>(buffer + iter);
        auto in_size = std::min(size, iter + buffer_.size()) - iter;
        auto out_size = lame_encode_buffer_interleaved(lame_.get(),
            const_cast<int16_t*>(ptr), in_size >> 2, buffer_.data(), buffer_.size());
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
  return std::make_unique<OutputMp3StreamImpl>(fname);
}

}  // namespace wav
