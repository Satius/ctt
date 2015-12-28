#ifndef CTT_UTIL_IMPL_H_
#define CTT_UTIL_IMPL_H_

#include <sstream>

namespace util {
namespace detail {

constexpr char kSpaceString[] = " ";

template<class T>
inline void JoinImpl(std::ostringstream& buf, const char*, const T& arg) {
  buf << arg;
}

template<class T, class... A>
inline void JoinImpl(std::ostringstream& buf, const char* sep, const T& arg, const A&... tail) {
  buf << arg << sep;
  JoinImpl(buf, sep, tail...);
}

}  // namespace detail
}  // namespace util

#endif  // CTT_UTIL_IMPL_H_
