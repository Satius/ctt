#include "util.h"
#include "mp3.h"
#include <fstream>

namespace {

struct OutputMp3StreamImpl : public mp3::IOutputMp3Stream {
 private:
  std::ofstream dst_;
  mp3::Info info_;

 public:
  OutputMp3StreamImpl(const std::string& fname) :
      dst_(fname) {
    if (!dst_) {
      const auto& message = util::Join("Can not open file", fname, "for writing");
      util::ThrowSystemError(message);
    }
  }

  void SetInfo(const mp3::Info& info) override {
    info_ = info;
  }

  std::size_t Write(const uint8_t* buffer, std::size_t size) override {
    dst_.write(reinterpret_cast<const char*>(buffer), size);
    if (!dst_) util::ThrowSystemError("File writing failed");
    return size;
  }
};

}  // namespace

namespace mp3 {

OutputMp3StreamPtr WriteMp3File(const std::string& fname) {
  return std::make_unique<OutputMp3StreamImpl>(fname);
}

}  // namespace wav
