#include "mp3.h"
#include "util.h"
#include "wav.h"
#include <iostream>
#include <vector>

namespace {

const char kDstExtension[] = "mp3";
const char kSrcExtension[] = "wav";
const int kBufferSize = 1024 * 1024;
const int kMp3Quality = 5;

}  // namespace

void HandleStreams(IInputStream& src, IOutputStream& dst) {
  try {
    std::vector<uint8_t> buffer(kBufferSize);
    for (;;) {
      auto size = src.Read(buffer.data(), buffer.size());
      if (!size) break;
      if (dst.Write(buffer.data(), size) != size)
        throw std::runtime_error("Failed to write compete data");
    }
  } catch (const std::exception&) {
    std::throw_with_nested(std::runtime_error("Streams handling failed"));
  }
}

void EncodeFile(const std::string& path) {
  try {
    if (!util::CheckExtension(path, kSrcExtension))
      throw std::runtime_error("Wrong source file extension");
    auto src = wav::ReadWavFile(path);
    const auto& src_info = src->GetInfo();
    mp3::Info dst_info = {src_info.num_channels, src_info.sample_rate, src_info.bits_per_sample, kMp3Quality};
    auto dst = mp3::WriteMp3File(util::ChangeExtension(path, kDstExtension));
    dst->SetInfo(dst_info);
    HandleStreams(*src, *dst);
  } catch (const std::exception&) {
    const auto& msg = util::Join("Failed to encode file", path);
    std::throw_with_nested(std::runtime_error(msg));
  }
}

int main(int argc, char** argv) {
  try {
    if (argc < 2) throw std::runtime_error("Wrong set of arguments");
    EncodeFile(argv[1]);
    return 0;
  } catch (const std::exception& ex) {
    int i = 0;
    util::UnwindNested(ex, [&i](const auto& op){
      std::cerr << "!" << std::string(i++ * 2 + 1, ' ') << op.what() << std::endl;
    });
    return 1;
  }
}
