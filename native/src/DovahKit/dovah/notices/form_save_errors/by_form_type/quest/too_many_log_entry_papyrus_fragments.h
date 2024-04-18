#pragma once
#include <cstdint>
#include <limits>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_type::quest {
   class too_many_log_entry_papyrus_fragments : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_log_entry_papyrus_fragments(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, std::numeric_limits<uint16_t>::max())
         {}
   };
}
#include "../../../_util.undef.h"