#ifndef CTT_UTIL_H_
#define CTT_UTIL_H_

#include "util_impl.h"

namespace util {

template<const char* sep = detail::kSpaceString, class... T>
inline std::string Join(const T&... args) {
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

}  // namespace util

#endif  // CTT_UTIL_H_
