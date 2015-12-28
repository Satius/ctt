#include "util.h"

namespace {

constexpr char kExtSeparator[] = ".";

}  // namespace

namespace util {

void ThrowSystemError(const std::string& msg) {
  try {
    throw std::system_error(errno, std::system_category());
  } catch (const std::exception&) {
    std::throw_with_nested(std::runtime_error(msg));
  }
}

bool CheckExtension(const std::string& fname, const std::string& ext) {
  auto sep = fname.find_last_of(kExtSeparator);
  return sep != std::string::npos &&
      !fname.compare(sep + 1, ext.size(), ext);
}

std::string ChangeExtension(const std::string& fname, const std::string& ext) {
  auto sep = fname.find_last_of(kExtSeparator);
  return Join<kExtSeparator>(fname.substr(0, sep), ext);
}

}  // namespace util
