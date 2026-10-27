#include "./can_load_form_data_of_type.h"
#include "dovah/form_data_function_tables/create_blank_instance.h"
#include "dovah/form_data_function_tables/load.h"
#include "dovah/form_type.h"

namespace dovah {
   extern bool can_load_form_data_of_type(form_type ft) {
      switch (ft) {
         case form_type::none:
         case form_type::file_header:
         case form_type::file_record_group:
         case form_type::setting:
         case form_type::toft:
         case form_type::alias:
         case form_type::reference_alias:
         case form_type::location_alias:
         case form_type::active_magic_effect:
            return false;
      }
      if (!form_data_function_tables::load::lookup(ft))
         return false;
      if (!form_data_function_tables::create_blank_instance::lookup(ft))
         return false;
      return true;
   }
}