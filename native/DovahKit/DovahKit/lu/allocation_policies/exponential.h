#pragma once
#include <bit>
#include <limits>

namespace lu::allocation_policies {
   class exponential {
      private:
         exponential() = delete;
         ~exponential() = delete;

      public:
         template<size_t MaxCapacity = std::numeric_limits<size_t>::max()>
         static constexpr size_t get_next_capacity(size_t new_size, size_t old_capacity) {
            if (new_size <= old_capacity)
               return old_capacity;
            size_t new_capacity = 0;
            if (std::has_single_bit(old_capacity)) {
               new_capacity = old_capacity << 1;
            } else {
               new_capacity = std::bit_ceil(old_capacity);
            }
            if (new_capacity < MaxCapacity)
               return new_capacity;

            return new_size;
         }
   };
}