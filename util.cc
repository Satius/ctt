#include "util.h"
#include <memory>
#include <system_error>

#ifdef _WIN32
#include <windows.h>
#else  // _WIN32
#include <dirent.h>
#include <unistd.h>
#endif  // _WIN32

namespace {

const char kDirSeparator[] = "/";
const char kExtSeparator[] = ".";

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
  return Join(kExtSeparator, fname.substr(0, sep), ext);
}

std::stack<std::string> EnumDir(const std::string& path, const std::function<bool(const std::string&)>& filter) {
  std::stack<std::string> result;
#ifdef _WIN32
  WIN32_FIND_DATA item;
  using PtrType = std::remove_pointer<HANDLE>::type;
  const auto& wildcard = Join(kDirSeparator, path, "*");
  std::unique_ptr<PtrType, decltype(&FindClose)> dir(FindFirstFile(wildcard.c_str(), &item), FindClose);
  if (dir.get() == INVALID_HANDLE_VALUE) {
    const auto& message = Join(kSpaceString, "Failed to open directory", path);
    util::ThrowSystemError(message);
  }
  do {
    if (filter(item.cFileName))
      result.push(Join(kDirSeparator, path, item.cFileName));
  } while (FindNextFile(dir.get(), &item));
#else  // _WIN32
  std::unique_ptr<DIR, decltype(&closedir)> dir(opendir(path.c_str()), closedir);
  if (!dir) {
      const auto& message = Join(kSpaceString, "Failed to open directory", path);
      util::ThrowSystemError(message);
  }
  for (dirent* item; (item = readdir(dir.get()));) {
    if (filter(item->d_name))
      result.push(Join(kDirSeparator, path, item->d_name));
  }
#endif
  return result;
}

int GetCoreCount() {
#ifndef _WIN32
  return sysconf(_SC_NPROCESSORS_ONLN);
#else  // _WIN32
  SYSTEM_INFO sysinfo;
  GetSystemInfo(&sysinfo);
  return sysinfo.dwNumberOfProcessors;
#endif  // _WIN32
}

}  // namespace util
