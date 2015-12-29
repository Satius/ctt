#ifndef CTT_THREAD_H_
#define CTT_THREAD_H_

#include "util.h"
#include <pthread.h>

// Obviously it's better to use <thread> and <mutex>
// Pthread is used to conform to test task description

namespace thread {

class MutexLocker : public util::NonCopyable {
 private:
  pthread_mutex_t* mutex_;

 public:
  MutexLocker(pthread_mutex_t* mutex);
  MutexLocker(MutexLocker&& other);
  ~MutexLocker();
};

}  // namespace thread

#endif  // CTT_THREAD_H_
