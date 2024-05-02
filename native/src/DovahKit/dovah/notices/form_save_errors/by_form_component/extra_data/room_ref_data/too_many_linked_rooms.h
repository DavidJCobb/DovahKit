#pragma once
#include <cstdint>
#include "../../../base_list_size_too_large_to_serialize_error.h"

#include "dovah/forms/components/extra_data/room_ref_data.h"

#include "../../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_component::extra_data::room_ref_data {
   class too_many_linked_rooms : public base_list_size_too_large_to_serialize_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         constexpr too_many_linked_rooms(form_stub& subject, size_t size)
         :
            base_list_size_too_large_to_serialize_error(subject, size, dovah::loaded_forms::components::extra::room_ref_data::max_linked_room_count)
         {}
   };
}
#include "../../../../_util.undef.h"