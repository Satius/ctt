#include "util.h"
#include "wav.h"
#include <fstream>

namespace {

wav::Info ReadWavInfo(std::ifstream& src) {
  return {};
}

class InputWavStreamImpl : public wav::IInputWavStream {
 private:
  std::ifstream src_;
  wav::Info info_;

 public:
  InputWavStreamImpl(const std::string& fname) :
      src_(fname), info_(ReadWavInfo(src_)) {
    if (!src_) {
      const auto& message = util::Join("Can not open file", fname, "for reading");
      util::ThrowSystemError(message);
    }
  }

  wav::Info GetInfo() const override {
    return info_;
  }

  std::size_t Read(uint8_t* buffer, std::size_t size) override {
    src_.read(reinterpret_cast<char*>(buffer), size);
    if (!src_ && !src_.eof()) util::ThrowSystemError("File reading failed");
    return src_.gcount();
  }
};

}  // namespace

namespace wav {

InputWavStreamPtr ReadWavFile(const std::string& fname) {
  return std::make_unique<InputWavStreamImpl>(fname);
}

}  // namespace wav
