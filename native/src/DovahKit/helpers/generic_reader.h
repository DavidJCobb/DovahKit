#pragma once
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "type_traits.h"

namespace cobb {
   namespace impl::generic_reader {
      template<typename T> concept IsLiteral = requires {
         requires (std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_enum_v<T>);
      };
      template<typename T> concept IsLiteralIsh = IsLiteral<T> || (std::is_bounded_array_v<T> && IsLiteral<std::remove_extent_t<T>>);
   }

   class generic_reader {
      protected:
         const void* _data = nullptr;
         size_t      _size = 0;
         size_t      _position = 0;

         inline const void* _at() const noexcept { return (const void*)((std::intptr_t)this->_data + this->_position); }

      public:
         generic_reader() {}
         generic_reader(const void* d, size_t s) : _data(d), _size(s) {}

         inline const void* data() const noexcept { return this->_data; }
         inline size_t size() const noexcept { return this->_size; }
         inline size_t position() const noexcept { return this->_position; }
         inline bool empty() const noexcept { return this->_data == nullptr || this->_size == 0; }

         inline const void* data_at(size_t p) const noexcept {
            if (p > this->_size)
               return nullptr;
            if (this->_data == nullptr)
               return nullptr;
            return (const void*)((std::intptr_t)this->_data + p);
         }

         void set_data(const void*, size_t);

         inline bool at_end() const noexcept { return this->_position == this->_size; }
         inline bool is_in_bounds(size_t s) const noexcept { return this->_position + s <= this->_size; }

         inline void skip(size_t s) {
            this->_position += s;
            if (this->_position >= this->_size)
               this->_position = this->_size;
         }

         #pragma region read
         inline bool read(void* buffer, size_t size) {
            if (!this->is_in_bounds(size))
               return false;
            memcpy(buffer, _at(), size);
            this->_position += size;
         }
         template<typename T> requires (impl::generic_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline bool read(T& field) const {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               if (!this->is_in_bounds(total_size))
                  return false;
               return this->unchecked_read(&field, total_size);
            } else {
               return this->read(&field, sizeof(T));
            }
         }
         #pragma endregion

         #pragma region unchecked_read
         inline bool unchecked_read(void* buffer, size_t size) {
            memcpy(buffer, _at(), size);
            this->_position += size;
         }
         template<typename T> requires (impl::generic_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline void unchecked_read(T& field) const {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               return this->unchecked_read(&field, total_size);
            } else {
               return this->unchecked_read(&field, sizeof(T));
            }
         }
         #pragma endregion
   };
}