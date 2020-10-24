#include "Shout.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Shout::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      #pragma region TESShout parents that get reset with each override
      this->name.reset();                // TESFullName
      this->description.reset();         // TESDescription
      this->menu_display_object.reset(); // BGSMenuDisplayObject
      this->equip_type.reset();          // BGSEquipType
      #pragma endregion
      //
      size_t current_word = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'FULL':
               subrecord.to_string(this->name);
               break;
            case 'MDOB':
               subrecord.read(this->menu_display_object);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::statik, *this->stub, this->menu_display_object)
               );
               break;
            case 'ETYP':
               subrecord.read(this->equip_type);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::equip_slot, *this->stub, this->equip_type)
               );
               break;
            case 'DESC':
               subrecord.to_string(this->description);
               break;
            case 'SNAM':
               if (current_word < this->words.size()) {
                  auto& entry = this->words[current_word];
                  subrecord.read(entry.word_of_power);
                  subrecord.read(entry.spell);
                  subrecord.read(entry.recoveryTime);
                  //
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::word_of_power, *this->stub, entry.word_of_power)
                  );
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     file_read_warning::warn_if_wrong_type(subrecord.signature(), form_type::spell, *this->stub, entry.spell)
                  );
               }
               ++current_word;
               break;
            default:
               intfc.log_load_warning(
                  file_read_warning::warn_about_unrecognized_subrecord(subrecord.signature(), *this->stub)
               );
               break;
         }
      }
      if (current_word != this->words.size()) {
         //
         // The game always assumes that SHOU will have three SNAMs. The Rule of One will not be properly 
         // applied if a SHOU override supplies fewer than three SNAMs. We should, of course, double-check 
         // this, by setting up tests with bad numbers of SNAM and using conditional breakpoints to see if 
         // the game fails to clear data.
         //
         file_read_warning warning;
         warning.code = notice_code::shout_has_wrong_word_count;
         warning.set_cause_form(*this->stub);
         warning.extra_integers[0] = current_word;
         //
         intfc.log_load_warning(warning);
      }
   }
   /*static*/ void Shout::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'MDOB': // menu display object
            case 'ETYP': // equip type
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'SNAM':
               if (subrecord.read(formID)) {
                  uib.add_outbound_reference(formID);
                  if (subrecord.read(formID)) {
                     uib.add_outbound_reference(formID);
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
   void Shout::setup(const file_load_order& load_order) noexcept {
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
      for (size_t i = 0; i < this->words.size(); ++i) {
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
      record.write_formID_subrecord('ETYP', this->equip_type, true);
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
      this->equip_type.clear_if(*this->stub, other);
      this->menu_display_object.clear_if(*this->stub, other);
      for (auto& word : this->words) {
         word.word_of_power.clear_if(*this->stub, other);
         word.spell.clear_if(*this->stub, other);
      }
   }
}