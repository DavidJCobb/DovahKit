#pragma once
#include <atomic>
#include <mutex>

namespace cobb {
   class lockable_bool {
      protected:
         mutable std::recursive_mutex mutex;
         std::atomic<bool> value  = false;
      public:
         lockable_bool() {}
         lockable_bool(bool b) : value(b) {}
         lockable_bool(const lockable_bool& b) : value(b) {}

         operator bool() const noexcept {
            return this->value;
         }
         lockable_bool& operator=(bool b) noexcept {
            mutex.lock();
            this->value = b;
            mutex.unlock();
            return *this;
         }

         inline void lock() noexcept { return this->mutex.lock(); }
         inline bool try_lock() noexcept { return this->mutex.try_lock(); }
         inline void unlock() noexcept { return this->mutex.unlock(); }
   };
}