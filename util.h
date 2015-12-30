#ifndef CTT_UTIL_H_
#define CTT_UTIL_H_

#include "util_impl.h"
#include <functional>
#include <stack>

namespace util {

const char kSpaceString[] = " ";

struct NonCopyable {
  NonCopyable() = default;
  NonCopyable(const NonCopyable&) = delete;
  NonCopyable& operator=(const NonCopyable&) = delete;
};

// Supposed to be template<const char* sep = detail::kSpaceString, class... T>
// Removed because constexpr implementation is broken in VS2015 compiler

template<class... T>
inline std::string Join(const char* sep, const T&... args) {
  std::ostringstream buf;
  detail::JoinImpl(buf, sep, args...);
  return buf.str();
}

template<class T>
inline void UnwindNested(const std::exception& op, const T& cb) {
  try {
    cb(op);
    std::rethrow_if_nested(op);
  } catch (const std::exception& ex) {
    UnwindNested(ex, cb);
  }
}

template<class T, std::size_t N>
inline constexpr std::size_t Length(const T(&)[N]) {
  return N;
}

void ThrowSystemError(const std::string& msg);
bool CheckExtension(const std::string& fname, const std::string& ext);
std::string ChangeExtension(const std::string& fname, const std::string& ext);
std::stack<std::string> EnumDir(const std::string& path, const std::function<bool(const std::string&)>& filter);
int GetCoreCount();

}  // namespace util

#endif  // CTT_UTIL_H_
