#include "thread.h"
#include "util.h"

namespace thread {

MutexLocker::MutexLocker(pthread_mutex_t* mutex) :
    mutex_(mutex) {
  if (pthread_mutex_lock(mutex_))
    util::ThrowSystemError("Failed to lock mutex");
}

MutexLocker::MutexLocker(MutexLocker&& other) :
    mutex_(other.mutex_) {
  other.mutex_ = nullptr;
}

MutexLocker::~MutexLocker() {
  if (mutex_) pthread_mutex_unlock(mutex_);
}

}  // namespace thread
