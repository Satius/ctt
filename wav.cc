#include "util.h"
#include "wav.h"

namespace {

wav::Info ReadWavInfo(std::ifstream& src) {
  return {};
}

}  // namespace

namespace wav {

InputWavStream::InputWavStream(const std::string& fname) :
    src_(fname), info_(ReadWavInfo(src_)) {
  if (!src_) {
    const auto& message = util::Join("Can not open file", fname, "for reading");
    util::ThrowSystemError(message);
  }
}

Info InputWavStream::GetInfo() const {
  return info_;
}

std::size_t InputWavStream::Read(uint8_t* buffer, std::size_t size) {
  src_.read(reinterpret_cast<char*>(buffer), size);
  if (!src_ && !src_.eof()) util::ThrowSystemError("File reading failed");
  return src_.gcount();
}

}  // namespace wav
