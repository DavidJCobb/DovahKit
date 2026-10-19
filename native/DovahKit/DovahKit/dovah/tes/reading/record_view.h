#pragma once
#include <cstdint>
#include "lu/concepts/instantiation_of/std_array.h"
#include "lu/memory/basic_buffer.h"
#include "../bytereader.h"
#include "../record_header.h"
#include "../serialization/can_be_copied.h"
#include "../serialization/serialized_bytecount_of.h"
#include "./impl/enforce_endianness.h"
namespace dovah {
   namespace tes {
      class file_view;
      class subrecord_view;
   }
   class managed_localized_string_use;
   class managed_form_use;
}

namespace dovah::tes {
   class record_view;
   class record_view : bytereader {
      friend file_view;
      public:
         using size_type = decltype(record_header::size);

      public:
         record_view(file_view& o) : owner(o) {}

      protected:
         file_view&    owner;
         record_header header;
         struct {
            size_type head = 0;
            size_type body = 0;
            size_type end  = 0;
         } file_offsets;

         // make these functions protected:
         using bytereader::set_needs_endianness_swap;

      public:
         constexpr operator bool() const noexcept { return this->header.signature != 0; }
         constexpr bool exists() const noexcept { return this->header.signature != 0; }
         
         constexpr record_flags flags() const noexcept { return this->header.flags; }
         constexpr uint32_t     formID() const noexcept { return this->header.formID; }
         constexpr lu::four_cc  signature() const noexcept { return this->header.signature; }
         constexpr size_type    size() const noexcept { return this->header.size; }
         constexpr uint16_t     version() const noexcept { return this->header.version; }

         constexpr const void* data() const noexcept { return this->_view.data(); }
         constexpr const void* data_at(size_type offset) const noexcept { return this->_view.data_at(offset); }
         
         constexpr size_type file_offset_of_header() const noexcept { return this->file_offsets.head; }
         constexpr size_type file_offset_of_body() const noexcept { return this->file_offsets.body; }
         constexpr size_type file_offset_of_end() const noexcept { return this->file_offsets.end; }

         const subrecord_view& get_current_subrecord() const noexcept;
         subrecord_view& get_current_subrecord() noexcept;
         subrecord_view& next_subrecord();
         lu::four_cc peek_next_subrecord_signature();
   };
}