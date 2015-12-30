#ifndef CTT_MP3_H_
#define CTT_MP3_H_

#include "stream.h"
#include <memory>
#include <string>

namespace mp3 {

struct Info {
  int num_channels;
  int sample_rate;
  int bits_per_sample;
  int quality;
};

struct IOutputMp3Stream : public IOutputStream {
  virtual void SetInfo(const Info& info) = 0;
};

using OutputMp3StreamPtr = std::unique_ptr<IOutputMp3Stream>;
OutputMp3StreamPtr WriteMp3File(const std::string& fname);

}  // namespace mp3

#endif  // CTT_MP3_H_
