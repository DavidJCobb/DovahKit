#pragma once

namespace lu::memory {
   class buffer_view {
      public:
         using size_type = size_t;

      protected:
         const void* _data = nullptr;
         size_type   _size = 0;

      public:
         constexpr buffer_view() {}
         constexpr buffer_view(const void*, size_type);

         constexpr const void* data() const noexcept;
         constexpr const void* data_at(size_type) const noexcept;
         constexpr bool empty() const noexcept;
         constexpr const void* end() const noexcept;
         constexpr size_type size() const noexcept;
   };
}

#include "./buffer_view.inl"