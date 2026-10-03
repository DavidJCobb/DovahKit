#pragma once
#include <cstdint>
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/memory/basic_buffer.h"
#include "../record_header.h"
#include "../serialization/can_be_copied.h"
#include "../serialization/serialized_bytecount_of.h"
#include "./impl/enforce_endianness.h"
namespace dovah {
   namespace tes {
      class file_view;
   }
   class managed_localized_string_use;
   class managed_form_use;
}

namespace dovah::tes {
   class record_view {
      friend file_view;
      public:
         using size_type = decltype(record_header::size);

      public:
         record_view(file_view& o) : owner(o) {}

      protected:
         file_view&    owner;
         record_header header;
         size_type     head_pos = 0; // position in the file (start of the record)
         size_type     body_pos = 0; // position in the file (start of the record body)
         size_type     end      = 0;

         lu::memory::basic_buffer buffer; // record body
         size_type offset = 0; // position within the record body

      protected:
         bool _needs_endianness_swap() const noexcept;

      public:
         constexpr operator bool() const noexcept { return this->header.signature != 0; }
         constexpr bool exists() const noexcept { return this->header.signature != 0; }

         const subrecord_view& get_current_subrecord() const noexcept;
         subrecord_view& get_current_subrecord() noexcept;
         subrecord_view& next_subrecord();
         lu::four_cc peek_next_subrecord_signature();
         
         constexpr record_flags flags() const noexcept { return this->header.flags; }
         constexpr uint32_t     formID() const noexcept { return this->header.formID; }
         constexpr lu::four_cc  signature() const noexcept { return this->header.signature; }
         constexpr size_type    size() const noexcept { return this->header.size; }
         constexpr uint16_t     version() const noexcept { return this->header.version; }

         constexpr const void* data() const noexcept { return this->buffer.data(); }
         constexpr const void* data_at(size_type offset) const noexcept { return this->buffer.data_at(offset); }
         
         constexpr bool is_in_bounds() const noexcept {
            return this->offset < this->buffer.size();
         }
         constexpr bool is_in_bounds(size_type room_for) const noexcept {
            return this->offset + room_for <= this->buffer.size();
         }
         
         constexpr size_type header_pos() const noexcept { return this->head_pos; }
         constexpr size_type stream_pos() const noexcept { return this->offset + this->body_pos; }
         constexpr size_type current_offset() const noexcept { return this->offset; } // how far into the (decompressed) data have we read?

         #pragma region Reading contents
            template<typename... Ts>
               requires (sizeof...(Ts) > 0 && (serialization::can_be_copied<Ts>::value && ...))
            bool read(Ts&... dst) {
               constexpr size_t total_size = serialization::serialized_bytecount_of<Ts>::value + ...;
               if (!this->is_in_bounds(total_size))
                  return false;
               this->unchecked_read(dst...);
               return true;
            }

            bool read_bytes(void* dst, size_t dst_size) {
               if (!this->is_in_bounds(dst_size))
                  return false;
               this->unchecked_read_bytes(dst, dst_size);
               return true;
            }

            void skip_bytes(size_type s) {
               this->offset += s;
               const auto s = this->buffer.size();
               if (this->offset > s)
                  this->offset = s;
            }

            template<typename T>
               requires serialization::can_be_copied<T>::value
            void unchecked_read(T& dst) {
               std::memcpy(&dst, this->buffer.data_at(this->offset), sizeof(dst));
               this->offset += sizeof(dst);
               impl::enforce_endianness(_needs_endianness_swap(), dst);
            }
            void unchecked_read(managed_localized_string_use&) = delete; // TODO
            void unchecked_read(managed_form_use&) = delete; // TODO

            template<typename... Ts>
               requires (sizeof...(Ts) > 1 && (serialization::can_be_copied<Ts>::value && ...))
            void unchecked_read(Ts&... dst) {
               (this->unchecked_read(dst), ...);
            }

            void unchecked_read_bytes(void* dst, size_t dst_size) {
               std::memcpy(dst, this->buffer.data_at(this->offset), dst_size);
               this->offset += dst_size;
            }
         #pragma endregion
   };
}