#pragma once
#include <cstdint>
#include <limits>
#include "../../unprefixed_string_is_too_long_to_serialize.h"

#include "dovah/forms/Race.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_type::race {
   class biped_object_name_is_too_long : public unprefixed_string_is_too_long_to_serialize {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr biped_object_name_is_too_long(form_stub& subject, size_t size, size_t which)
         :
            unprefixed_string_is_too_long_to_serialize(subject, size, dovah::loaded_forms::Race::max_biped_object_name_length, 'NAME'),
            which(which)
         {}

         size_t which;
   };
}
#include "../../../_util.undef.h"