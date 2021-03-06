#include "package_location.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void package_location::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      this->detail.form.unmanaged_set(nullptr); // prevent dangling pointers if a form contains multiple subrecords providing contradictory values for the same package location
      //
      subrecord.read(this->type);
      switch (this->type) {
         case package_location_type::in_cell:
            subrecord.read(this->detail.form);
            intfc.log_load_warning(
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::cell, intfc.target_stub, this->detail.form)
            );
            break;
         case package_location_type::near_reference:
            subrecord.read(this->detail.form);
            intfc.log_load_warning( // TODO: xEdit definitions claim that DOOR is valid here, but no other base forms. seems suspect imo but can we verify it?
               detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, this->detail.form)
            );
            break;
         case package_location_type::near_linked_reference: // KYWD
            subrecord.read(this->detail.form);
            intfc.log_load_warning(
               detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::keyword, intfc.target_stub, this->detail.form)
            );
            break;
         case package_location_type::object_id:
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
   void package_location::clone_from(const package_location& other, loaded_forms::Form& my_owner) noexcept {
      this->type = other.type;
      this->detail.form.set(my_owner, other.detail.form);
      this->detail.object_type = other.detail.object_type;
      this->detail.alias_id    = other.detail.alias_id;
      this->detail.padding     = other.detail.padding;
      this->radius = other.radius;
   }
   void package_location::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->detail.form.clear_if(my_owner, target);
   }
   void package_location::clear(loaded_forms::Form& my_owner) {
      this->detail.form.set(my_owner, nullptr);
      this->detail.object_type =  0;
      this->detail.alias_id    = -1;
      this->detail.padding     =  0;
      this->radius = 0;
   }

   #pragma region package_location::use_info_state
   void package_location::use_info_state::generate_use_info(tes_subrecord_reader& subrecord) {
      package_location_type t;
      if (subrecord.read(t)) {
         switch (t) {
            case package_location_type::near_reference:
            case package_location_type::in_cell:
            case package_location_type::object_id:
            case package_location_type::near_linked_reference:
               subrecord.read(this->form);
               break;
         }
      }
   }
   void package_location::use_info_state::clear() {
      this->form = 0;
   }
   void package_location::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->form)
         uib.add_outbound_reference(this->form);
   }
   #pragma endregion
}