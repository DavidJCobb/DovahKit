#pragma once
#include <cstdint>
#include <limits>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_type::quest {
   //
   // We can't save the number of aliases whose data is in the VMAD.
   //
   class too_many_scripted_aliases : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_scripted_aliases(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, std::numeric_limits<uint16_t>::max())
         {}
   };
}
#include "../../../_util.undef.h"