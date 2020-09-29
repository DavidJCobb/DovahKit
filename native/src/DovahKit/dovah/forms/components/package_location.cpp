#include "package_location.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void package_location::load(tes_subrecord_reader& subrecord) {
      subrecord.read(this->type);
      switch (this->type) {
         case package_location_type::near_reference:
         case package_location_type::in_cell:
         case package_location_type::object_id:
         case package_location_type::near_linked_reference: // KYWD
            subrecord.read(this->detail.form);
            break;
         case package_location_type::reference_alias:
         case package_location_type::location_alias:
            subrecord.read(this->detail.alias_id);
            break;
         case package_location_type::near_package_start_location:
         case package_location_type::near_editor_location:
         case package_location_type::at_package_location:
         case package_location_type::unknown_10:
         case package_location_type::unknown_11:
         case package_location_type::near_self:
            subrecord.read(this->detail.padding);
            break;
         case package_location_type::object_type:
            subrecord.read(this->detail.object_type);
            break;
      }
      subrecord.read(this->radius);
   }
   /*static*/ void package_location::generateUseInfo(tes_subrecord_reader& subrecord, form_stub* stub) {
      package_location_type t;
      if (subrecord.read(t)) {
         form_id_t formID;
         switch (t) {
            case package_location_type::near_reference:
            case package_location_type::in_cell:
            case package_location_type::object_id:
            case package_location_type::near_linked_reference:
               if (subrecord.read(formID) && formID)
                  stub->add_outbound_reference(formID);
               break;
         }
      }
   }
   void package_location::save(tes_subrecord_writer& subrecord) {
      subrecord.write(this->type);
      switch (this->type) {
         case package_location_type::near_reference:
         case package_location_type::in_cell:
         case package_location_type::object_id:
         case package_location_type::near_linked_reference: // KYWD
            subrecord.write(this->detail.form);
            break;
         case package_location_type::reference_alias:
         case package_location_type::location_alias:
            subrecord.write(this->detail.alias_id);
            break;
         case package_location_type::near_package_start_location:
         case package_location_type::near_editor_location:
         case package_location_type::at_package_location:
         case package_location_type::unknown_10:
         case package_location_type::unknown_11:
         case package_location_type::near_self:
            subrecord.write(this->detail.padding);
            break;
         case package_location_type::object_type:
            subrecord.write(this->detail.object_type);
            break;
      }
      subrecord.write(this->radius);
   }
}