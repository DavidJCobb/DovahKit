/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#pragma once
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include "type_traits/is_literal.h"
#include "type_traits/is_std_array.h"

namespace cobb {
   namespace impl::generic_reader {
      template<typename T> concept IsLiteralIsh = cobb::is_literal<T> || (std::is_bounded_array_v<T> && cobb::is_literal<std::remove_extent_t<T>>);
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
            return true;
         }
         template<typename T> requires (impl::generic_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline bool read(T& field) {
            if constexpr (cobb::is_std_array<T>) {
               constexpr size_t total_size = sizeof(T::value_type) * field.size();
               if (!this->is_in_bounds(total_size))
                  return false;
               return this->unchecked_read(&field, total_size);
            } else {
               return this->read(&field, sizeof(T));
            }
         }

         template<typename S> requires std::convertible_to<S, size_t>
         bool read_prefixed_string(std::string& field) {
            S size;
            if (!this->read(size))
               return false;
            field.resize(size);
            if (size) {
               if (!this->read(field.data(), size))
                  return false;
               if (field.back() == '\00')
                  field.resize(size - 1);
            }
            return true;
         }
         #pragma endregion

         #pragma region unchecked_read
         inline bool unchecked_read(void* buffer, size_t size) {
            memcpy(buffer, _at(), size);
            this->_position += size;
         }
         template<typename T> requires (impl::generic_reader::IsLiteralIsh<T> || cobb::is_std_array<T>) inline void unchecked_read(T& field) {
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