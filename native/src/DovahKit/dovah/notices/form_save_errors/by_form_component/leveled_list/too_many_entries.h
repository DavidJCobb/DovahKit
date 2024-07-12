#pragma once
#include <cstdint>
#include "../../base_list_size_too_large_to_serialize_error.h"

#include "dovah/forms/components/leveled_list.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_component::leveled_list {
   class too_many_entries : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_entries(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, dovah::loaded_forms::components::leveled_list::max_entry_count)
         {}
   };
}
#include "../../../_util.undef.h"