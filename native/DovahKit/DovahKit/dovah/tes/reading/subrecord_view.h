#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "lu/concepts/integer.h"
#include "lu/type_aliases/uint_of_size.h"
#include "./record_view.h"
namespace dovah::tes {
   class file_view;
}

namespace dovah::tes {
   class subrecord_view {
      friend file_view;
      friend record_view;
      public:
         using size_type = uint32_t;

      public:
         subrecord_view(file_view& o) : _owner(o) {}

      protected:
         file_view&  _owner;
         lu::four_cc _signature = 0;
         size_type   _size      = 0;
         size_type   _body_pos  = 0; // start of body within containing record

      public:
         constexpr operator bool() const noexcept { return this->_signature != 0; }
         constexpr bool exists() const noexcept { return this->_signature != 0; }

         const record_view& get_containing_record() const;
         record_view& get_containing_record();
         
         constexpr const void* data() const noexcept { return this->get_containing_record().data_at(this->_body_pos); }
         constexpr auto      signature() const noexcept { return this->_signature; }
         constexpr size_type size() const noexcept { return this->_size; }

         constexpr size_type body_pos_in_record() const noexcept { return this->_body_pos; }
         constexpr size_type end_pos_in_record() const noexcept { return this->_body_pos + this->_size; }
         constexpr size_type current_offset() const noexcept { return this->get_containing_record().current_offset() - this->_body_pos; }
         
         constexpr bool is_in_bounds() const noexcept {
            return this->current_offset() < this->_size;
         }
         constexpr bool is_in_bounds(size_type room_for) const noexcept {
            return this->current_offset() + room_for <= this->_size;
         }

         #pragma region Reading contents
            template<typename... Ts> requires (sizeof...(Ts) > 0)
            bool read(Ts&... dst) {
               return this->get_containing_record().read(dst...);
            }

            void skip_bytes(size_type s) {
               this->get_containing_record().skip_bytes(s);
            }

            template<typename... Ts> requires (sizeof...(Ts) > 0)
            void unchecked_read(Ts&... dst) {
               this->get_containing_record().unchecked_read(dst...);
            }

            // Reads the full subrecord, even if its current offset is not 0.
            // Moves the view to the end of the subrecord.
            void read_all_to_string(std::string&);

            template<typename PrefixType>
               requires (lu::concepts::integer<PrefixType> && std::is_unsigned_v<PrefixType>)
            bool read_length_prefixed_string(std::string& dst) {
               PrefixType size = 0;
               if (!read(size))
                  return false;
               if (!is_in_bounds(size))
                  return false;
               auto& record = this->get_containing_record();
               dst.assign(record.data_at(record.current_offset()), size);
               record.skip_bytes(size);
               return true;
            }
         #pragma endregion
   };
}