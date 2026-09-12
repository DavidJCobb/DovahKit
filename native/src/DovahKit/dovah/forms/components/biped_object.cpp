#include "biped_object.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   void biped_object::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
      uint32_t keywordSize = 0;
      form_reference_t formID;
      switch (subrecord.signature()) {
         case subrecord_signature_deprecated:
            subrecord.read(this->first_person_slots);
            subrecord.read(this->flags);
            subrecord.skip_bytes(3);
            subrecord.read(this->armor_type);
            break;
         case subrecord_signature_modern:
            subrecord.read(this->first_person_slots);
            subrecord.read(this->armor_type);
            break;
      }
   }
   /*static*/ void biped_object::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      return;
   }
   void biped_object::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      //
      // Current versions of the CK only write BOD2; it seems BODT is deprecated.
      //
      if (this->flags != 0) {
         auto& BODT = record.open_next_subrecord(subrecord_signature_deprecated);
         BODT.reserve_more(0x0C);
         BODT.write(this->first_person_slots);
         BODT.write(this->flags);
         BODT.skip_bytes(3);
         BODT.write(this->armor_type);
         BODT.close();
      } else {
         auto& BOD2 = record.open_next_subrecord(subrecord_signature_modern);
         BOD2.reserve_more(0x08);
         BOD2.write(this->first_person_slots);
         BOD2.write(this->armor_type);
         BOD2.close();
      }
   }
   void biped_object::clear(loaded_forms::Form& my_owner) noexcept {
      this->first_person_slots = 0;
      this->flags = 0;
      this->armor_type = armor_type::clothing;
   }
   void biped_object::clone_from(const biped_object& other, loaded_forms::Form& my_owner) noexcept {
      *this = other;
   }
   void biped_object::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      return;
   }
}