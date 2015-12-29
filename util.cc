#include "util.h"
#include <dirent.h>
#include <unistd.h>
#include <memory>

namespace {

constexpr char kDirSeparator[] = "/";
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

std::stack<std::string> EnumDir(const std::string& path, const std::function<bool(const std::string&)>& filter) {
  std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), closedir);
  if (!dir) {
    const auto& message = Join("Failed to open directory", path);
    util::ThrowSystemError(message);
  }
  std::stack<std::string> result;
  for (dirent* item; (item = readdir(dir.get()));) {
    if (filter(item->d_name))
      result.push(Join<kDirSeparator>(path, item->d_name));
  }
  return result;
}

int GetCoreCount() {
  return sysconf(_SC_NPROCESSORS_ONLN);
}

}  // namespace util
