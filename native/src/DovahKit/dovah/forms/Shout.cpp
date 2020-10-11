#include "Shout.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Shout::load(tes_record_reader& record) {
      Form::load(record);
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MDOB':
               subrecord.read(this->menu_display_object);
               break;
            case 'DESC':
               subrecord.to_string(this->description);
               break;
            case 'SNAM':
               {
                  Word& entry = this->words.emplace_back();
                  subrecord.read(entry.word_of_power);
                  subrecord.read(entry.spell);
                  subrecord.read(entry.recoveryTime);
               }
               break;
         }
      }
   }
   /*static*/ void Shout::generateUseInfo(tes_record_reader& record, form_stub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'MDOB': // looping sound (e.g. nirnroot bell)
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'SNAM':
               if (subrecord.read(formID)) {
                  stub->add_outbound_reference(formID);
                  if (subrecord.read(formID)) {
                     stub->add_outbound_reference(formID);
                     // and then a four-byte float, which we can ignore
                  }
               }
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'DESC': // description
               break;
         }
      }
   }
   bool Shout::_clone_impl(Form* out) const noexcept {
      auto copy = dynamic_cast<Shout*>(out);
      if (!copy)
         return false;
      // NOTE: Form::clone already took care of the form flags, including the "treat as power" flag.
      copy->name        = this->name;
      copy->description = this->description;
      copy->menu_display_object.set(*copy->stub, this->menu_display_object);
      //
      size_t size = this->words.size();
      assert(copy->words.empty() && "We should be working with a newly-created form. If this isn't empty, then we need to clear out the form IDs already inside via (set) calls; simply resizing/clearing the vector and destroying form_id_ts will fail to clean up already-existing use info.");
      copy->words.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& word = copy->words[i];
         auto& from = this->words[i];
         word.word_of_power.set(*copy->stub, from.word_of_power);
         word.spell.set(*copy->stub, from.spell);
         word.recoveryTime = from.recoveryTime;
      }
      return true;
   }
   bool Shout::_save_impl(tes_record_writer& record) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& MDOB = record.open_next_subrecord('MDOB');
      MDOB.write(this->menu_display_object);
      MDOB.close();
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->description);
      DESC.close();
      for (auto& word : this->words) {
         auto& SNAM = record.open_next_subrecord('SNAM');
         SNAM.write(word.word_of_power);
         SNAM.write(word.spell);
         SNAM.write(word.recoveryTime);
         SNAM.close();
      }
      return true;
   }
   void Shout::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->menu_display_object.clear_if(*this->stub, other);
      for (auto& word : this->words) {
         word.word_of_power.clear_if(*this->stub, other);
         word.spell.clear_if(*this->stub, other);
      }
   }
}