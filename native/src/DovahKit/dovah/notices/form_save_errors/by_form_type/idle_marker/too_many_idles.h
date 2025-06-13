#pragma once
#include <cstdint>
#include <limits>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_type::idle_marker {
   class too_many_idles : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_idles(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, std::numeric_limits<uint8_t>::max())
         {}
   };
}
#include "../../../_util.undef.h"