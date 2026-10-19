#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "lu/concepts/integer.h"
#include "../bytereader.h"
namespace dovah::tes {
   class file_view;
   class record_view;
}

namespace dovah::tes {
   class subrecord_view : public bytereader {
      friend file_view;
      friend record_view;
      public:
         using size_type = uint32_t;

      public:
         subrecord_view(file_view& o) : _owner(o) {}

      protected:
         file_view&  _owner;
         lu::four_cc _signature = 0;
         size_type   _body_pos  = 0; // start of body within containing record

         // make these functions protected:
         using bytereader::set_needs_endianness_swap;

      public:
         constexpr operator bool() const noexcept { return this->_signature != 0; }
         constexpr bool exists() const noexcept { return this->_signature != 0; }

         const record_view& get_containing_record() const;
         record_view& get_containing_record();
         
         constexpr const void* data() const noexcept { return this->_view.data(); }
         constexpr const void* data_at(size_type s) const noexcept { return this->_view.data_at(s); }
         constexpr auto      signature() const noexcept { return this->_signature; }
         constexpr size_type size() const noexcept { return this->_view.size(); }

         constexpr size_type body_pos_in_record() const noexcept { return this->_body_pos; }
         constexpr size_type end_pos_in_record() const noexcept { return this->_body_pos + this->size(); }

         #pragma region Reading contents
            template<typename PrefixType>
               requires (lu::concepts::integer<PrefixType> && std::is_unsigned_v<PrefixType>)
            bool read_length_prefixed_string(std::string& dst) {
               PrefixType size = 0;
               if (!read(size))
                  return false;
               if (!is_in_bounds(size))
                  return false;
               unchecked_read_bytes(dst.data(), dst.size());
               return true;
            }

            void read_all_to_string(std::string& dst);
         #pragma endregion
   };
}