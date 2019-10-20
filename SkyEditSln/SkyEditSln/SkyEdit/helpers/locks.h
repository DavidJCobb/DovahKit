#pragma once

namespace cobb {
   template<typename T> class shared_lock_guard {
      private:
         T& lock;
      public:
         shared_lock_guard(T& l) : lock(l) { l.lock_shared(); }
         ~shared_lock_guard() { this->lock.unlock_shared(); }
   };
}
