#pragma once
#include <limits>

namespace lu::allocation_policies {
   class one_at_a_time {
      private:
         one_at_a_time() = delete;
         ~one_at_a_time() = delete;

      public:
         template<size_t MaxCapacity = std::numeric_limits<size_t>::max()>
         static constexpr size_t get_next_capacity(size_t new_size, size_t old_capacity) {
            if (new_size <= old_capacity)
               return old_capacity;
            return new_size;
         }
   };
}