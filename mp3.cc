#include "util.h"
#include "mp3.h"

namespace mp3 {

OutputMp3Stream::OutputMp3Stream(const std::string& fname) :
    dst_(fname) {
  if (!dst_) {
    const auto& message = util::Join("Can not open file", fname, "for writing");
    util::ThrowSystemError(message);
  }
}

void OutputMp3Stream::SetInfo(const Info& info) {
  info_ = info;
}

std::size_t OutputMp3Stream::Write(const uint8_t* buffer, std::size_t size) {
  dst_.write(reinterpret_cast<const char*>(buffer), size);
  if (!dst_) util::ThrowSystemError("File writing failed");
  return size;
}

}  // namespace wav
