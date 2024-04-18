#pragma once
#include <cstdint>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "dovah/forms/components/destruction.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_component::destruction {
   class too_many_stages : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_stages(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, dovah::loaded_forms::components::destruction_stage_data::max_stage_count)
         {}
   };
}
#include "../../../_util.undef.h"