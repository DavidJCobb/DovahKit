#include "./property_object_value.h"
#include "../../_common_cpp.h"
#include "./attachment_header.h"

namespace dovah::loaded_forms::components::papyrus {
   bool property_object_value::load(const attachment_header& header, tes_subrecord_reader& subrecord) {
      if (!subrecord.is_in_bounds(sizeof(this->always_zero) + sizeof(this->alias_id) + sizeof(bare_form_id_t)))
         return false;
      if (header.object_format == 2) {
         subrecord.unchecked_read(this->always_zero);
         subrecord.unchecked_read(this->alias_id);
         subrecord.unchecked_read(this->form);
      } else {
         subrecord.unchecked_read(this->form);
         subrecord.unchecked_read(this->alias_id);
         subrecord.unchecked_read(this->always_zero);
      }
      return true;
   }
   bool property_object_value::save(const attachment_header& header, tes_subrecord_writer& subrecord) const noexcept {
      if (header.object_format == 2) {
         subrecord.write(this->always_zero);
         subrecord.write(this->alias_id);
         subrecord.write(this->form);
      } else {
         subrecord.write(this->form);
         subrecord.write(this->alias_id);
         subrecord.write(this->always_zero);
      }
      return true;
   }
   void property_object_value::clone_from(const property_object_value& other, loaded_forms::Form& owner) noexcept {
      this->form.set(owner, other.form);
      this->alias_id    = other.alias_id;
      this->always_zero = other.always_zero;
   }
   void property_object_value::clear(loaded_forms::Form& owner) {
      this->form.set(owner, nullptr);
      this->alias_id = -1;
   }
   void property_object_value::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->form.clear_if(my_owner, target);
   }
}