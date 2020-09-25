#include "package_event_dialogue.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::components {
   bool package_event_dialogue::load(tes_record_reader& record) {
      //
      // The game reads subrecords indefinitely, stopping only after reading TNAM or 
      // PDTO. It will simply skip any unrecognized subrecords up to that point.
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'INAM':
               record.next_subrecord().read(this->idle);
               break;
            case 'TNAM':
               this->type = topic_type::ref;
               record.next_subrecord().read(this->topic);
               return true; // TESPackage::Data::Load aborts after reading TNAM
            case 'PDTO':
               {
                  auto& subrecord = record.next_subrecord();
                  subrecord.read(this->type);
                  if (this->type == topic_type::ref)
                     subrecord.read(this->topic);
                  else if (this->type == topic_type::subtype)
                     subrecord.read(this->topic_subtype);
               }
               return true; // TESPackage::Data aborts after reading PDTO
         }
      }
      return false;
   }
   /*static*/ void package_event_dialogue::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      package_event_dialogue temp;
      temp.load(record); // TODO: we have no way to signal errors re: malformed data
      if (temp.idle)
         stub->add_outbound_reference(temp.idle);
      if (temp.topic)
         stub->add_outbound_reference(temp.topic);
   }
   void package_event_dialogue::save(tes_record_writer& record) {
      if (this->idle)
         record.write_formID_subrecord('INAM', this->idle);
      auto& PDTO = record.open_next_subrecord('PDTO');
      PDTO.write(this->type);
      switch (this->type) {
         case topic_type::ref:
            PDTO.write(this->topic);
            break;
         case topic_type::subtype:
            PDTO.write(this->topic_subtype);
            break;
      }
      PDTO.close();
   }

   bool package_event_dialogue::empty() const noexcept {
      if (this->idle)
         return false;
      if (this->topic || this->topic_subtype)
         return false;
      return true;
   }
}