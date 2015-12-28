#ifndef CTT_MP3_H_
#define CTT_MP3_H_

#include "stream.h"
#include <fstream>

namespace mp3 {

struct Info {
};

class OutputMp3Stream : public IOutputStream {
 private:
  std::ofstream dst_;
  Info info_;

 public:
  OutputMp3Stream(const std::string& fname);
  void SetInfo(const Info& info);
  std::size_t Write(const uint8_t* buffer, std::size_t size) override;
};

}  // namespace mp3

#endif  // CTT_MP3_H_
