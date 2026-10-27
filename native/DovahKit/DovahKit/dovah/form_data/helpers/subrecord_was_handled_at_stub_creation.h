#pragma once
#include "lu/integral_character_code.h"

namespace dovah::form_data_helpers {
   // Keep in synch with `load_order_processes::build_form_stubs_from_file::impl::extract_high_value_subrecords_for_stub`, 
   // but only for those subrecord signatures that are dealt with for *all* form types.
   inline bool subrecord_was_handled_at_stub_creation(lu::four_cc signature) {
      switch (signature) {
         case 'EDID':
            return true;
      }
      return false;
   }
}