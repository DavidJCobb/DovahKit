#pragma once
#include <limits>

namespace lu::allocation_policies {
   template<size_t Count>
   class incremental {
      public:
         static constexpr const size_t increment = Count;

      private:
         incremental() = delete;
         ~incremental() = delete;

      public:
         template<size_t MaxCapacity = std::numeric_limits<size_t>::max()>
         static constexpr size_t get_next_capacity(size_t new_size, size_t old_capacity) {
            if (new_size <= old_capacity)
               return old_capacity;
            size_t rem = old_capacity % increment;
            if (rem) {
               old_capacity -= rem;
            }
            size_t new_capacity = old_capacity += increment;
            if (new_capacity < MaxCapacity)
               return new_capacity;
            return new_size;
         }
   };
}