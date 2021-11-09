#pragma once
#include <array>
#include <type_traits>
#include "_vulkan.h"

namespace vulkanDK {
   class physical_device;
   class surface;
   class surface_renderer;

   class queue_family_info {
      public:
         using queue_index_t = uint32_t;

      protected:
         void set(queue_index_t& entry, queue_index_t value);

      public:
         union {
            struct {
               queue_index_t graphics;
               queue_index_t presentation;
            };
            std::array<queue_index_t, 2> list;
         } families;
         uint8_t mask = 0;

         static constexpr size_t         unique_family_count = std::tuple_size_v<decltype(families.list)>;
         static constexpr decltype(mask) all_mask_bits_set   = (1 << unique_family_count) - 1;

         queue_family_info(surface_renderer&);
         queue_family_info(const physical_device&, const surface&);
         bool has(const queue_index_t& entry) const noexcept;

         inline bool has_index(size_t i) const noexcept {
            return (this->mask & (1 << i)) != 0;
         }
         
         static_assert(sizeof(families) == sizeof(families.list), "The array doesn't include all families. Did you add some without increasing the array length to match?");
         static_assert(sizeof(mask) * 8 >= std::bit_width(unique_family_count), "The mask type is not large enough to track all values. Make it bigger.");
   };
}