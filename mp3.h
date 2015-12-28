#ifndef CTT_MP3_H_
#define CTT_MP3_H_

#include "stream.h"
#include <memory>

namespace mp3 {

struct Info {
};

struct IOutputMp3Stream : public IOutputStream {
  virtual void SetInfo(const Info& info) = 0;
};

using OutputMp3StreamPtr = std::unique_ptr<IOutputMp3Stream>;
OutputMp3StreamPtr WriteMp3File(const std::string& fname);

}  // namespace mp3

#endif  // CTT_MP3_H_
