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
               subrecord.read(this->menuDisplayObjectID);
               break;
            case 'DESC':
               subrecord.to_string(this->description);
               break;
            case 'SNAM':
               {
                  Word& entry = this->words.emplace_back();
                  subrecord.read(entry.wordOfPowerID);
                  subrecord.read(entry.spellID);
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
      copy->menuDisplayObjectID.set(copy->stub, this->menuDisplayObjectID);
      //
      size_t size = this->words.size();
      assert(copy->words.empty() && "We should be working with a newly-created form. If this isn't empty, then we need to clear out the form IDs already inside via (set) calls; simply resizing/clearing the vector and destroying form_id_ts will fail to clean up already-existing use info.");
      copy->words.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& word = copy->words[i];
         auto& from = this->words[i];
         word.wordOfPowerID.set(copy->stub, from.wordOfPowerID);
         word.spellID.set(copy->stub, from.spellID);
         word.recoveryTime = from.recoveryTime;
      }
      return true;
   }
   bool Shout::_save_impl(tes_record_writer& record) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& MDOB = record.open_next_subrecord('MDOB');
      MDOB.write(this->menuDisplayObjectID);
      MDOB.close();
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->description);
      DESC.close();
      for (auto& word : this->words) {
         auto& SNAM = record.open_next_subrecord('SNAM');
         SNAM.write(word.wordOfPowerID);
         SNAM.write(word.spellID);
         SNAM.write(word.recoveryTime);
         SNAM.close();
      }
      return true;
   }
}