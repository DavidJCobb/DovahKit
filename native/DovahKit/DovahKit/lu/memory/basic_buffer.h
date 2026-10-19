#pragma once

namespace lu::memory {
   class basic_buffer {
      private:
         void*  _data = nullptr;
         size_t _size = 0;

      public:
         constexpr basic_buffer() {}
         ~basic_buffer();

         #pragma region Copy/move
            basic_buffer(const basic_buffer&);
            basic_buffer& operator=(const basic_buffer&);

            basic_buffer(basic_buffer&&) noexcept;
            basic_buffer& operator=(basic_buffer&&) noexcept;
         #pragma endregion

         constexpr const void* data() const noexcept;
         constexpr void* data() noexcept;
         constexpr bool empty() const noexcept;
         constexpr size_t size() const noexcept;

         constexpr const void* data_at(size_t offset) const noexcept;
         constexpr void* data_at(size_t offset) noexcept;

         void clear();
         void resize(size_t);
   };
}

#include "./basic_buffer.inl"