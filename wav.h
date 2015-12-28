#ifndef CTT_WAV_H_
#define CTT_WAV_H_

#include "stream.h"
#include <fstream>

namespace wav {

struct Info {
};

class InputWavStream : public IInputStream {
 private:
  std::ifstream src_;
  Info info_;

 public:
  InputWavStream(const std::string& fname);
  Info GetInfo() const;
  std::size_t Read(uint8_t* buffer, std::size_t size) override;
};

}  // namespace wav

#endif  // CTT_WAV_H_
