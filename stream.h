#ifndef CTT_STREAM_H_
#define CTT_STREAM_H_

#include <cstdint>

struct IInputStream {
  virtual std::size_t Read(uint8_t* buffer, std::size_t size) = 0;
  virtual ~IInputStream() = default;
};

struct IOutputStream {
  virtual std::size_t Write(const uint8_t* buffer, std::size_t size) = 0;
  virtual ~IOutputStream() = default;
};

#endif  // CTT_STREAM_H_
