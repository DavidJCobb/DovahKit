#pragma once
#include <cstdint>
#include <optional>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::extra_data {
   class room_ref_data_swallowed_subrecord : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class expected_field_type {
            lighting_template,
            imagespace,
            linked_room
         };

      public:
         constexpr room_ref_data_swallowed_subrecord(
            form_stub& subject,
            expected_field_type expected_field,
            uint32_t signature_seen
         )
         :
            base_form_load_warning(subject),
            expected_field(expected_field),
            signature_seen(signature_seen)
         {}
         
         expected_field_type expected_field;
         uint32_t signature_seen = 0;
         
         std::optional<size_t> is_nth_linked_room;
   };
}
#include "../../../_util.undef.h"