#include "./form_id.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/utils/form_ids/record_id_to_form_id.h"

namespace dovah {
   void form_id::unchecked_stream(tes::subrecord_view& subrecord) {
      subrecord.unchecked_read(this->wrapped_integral);
      const auto fixup = utils::form_ids::record_id_to_form_id(subrecord.get_source_file(), this->wrapped_integral);
      if (fixup.has_value()) {
         this->wrapped_integral = fixup.value();
      } else {
         this->wrapped_integral = 0;
      }
   }
}

// Compile-time sanity checks:
#include "lu/bytestreams/customization/serialized_bytecount.h"
static_assert(sizeof(dovah::form_id)  == sizeof(dovah::bare_form_id));
static_assert(alignof(dovah::form_id) == alignof(dovah::bare_form_id));
static_assert(lu::bytestreams::serialized_bytecount_is_statically_known<dovah::tes::subrecord_view, dovah::form_id>);
static_assert(lu::bytestreams::serialized_bytecount<dovah::tes::subrecord_view, dovah::form_id>::value == sizeof(dovah::bare_form_id));