#include "Shout.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Shout::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      #pragma region TESShout parents that get reset with each override
      this->name.reset();        // TESFullName
      this->description.reset(); // TESDescription
      this->menu_display_object.set(*this, nullptr); // BGSMenuDisplayObject
      this->equip_type.set(*this, nullptr);          // BGSEquipType
      //
      // The words of a shout don't get reset in ClearData or InitializeData, presumably 
      // because Bethesda expects  there to always be exactly  three of them. This means 
      // that an override with fewer than three  words will cause the latter words of an 
      // overridden record to "leak through."
      //
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
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::statik, this->stub, this->menu_display_object)
               );
               break;
            case 'ETYP':
               subrecord.read(this->equip_type);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::equip_slot, this->stub, this->equip_type)
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
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::word_of_power, this->stub, entry.word_of_power)
                        .set_subrecord_index(current_word)
                  );
                  intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                     detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::spell, this->stub, entry.spell)
                        .set_subrecord_index(current_word)
                  );
               }
               ++current_word;
               break;
            default:
               intfc.log_load_warning(
                  detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
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
         detailed_notice warning;
         warning.code = notice_code::shout_has_wrong_word_count;
         warning.set_cause_form(this->stub);
         warning.extra_integers[0] = current_word;
         //
         intfc.log_load_warning(warning);
      }
   }
   /*static*/ void Shout::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      int word_count = 0;
      //
      if (!uib.is_final_file()) {
         //
         // There is no data in this form type that is coalesced across multiple files. However, 
         // shout data defined in overridden records can "leak through" into the final form if the 
         // winning override doesn't define enough words.
         //
         // The use info builder interface gives us room to store some generic form IDs. In this 
         // case, we'll store three words' worth of form IDs, such that if the winning record 
         // doesn't supply enough words, we'll know what form IDs would "leak through" from the 
         // overridden records; we can then add outbound references to those form IDs.
         //
         while (auto& subrecord = record.next_subrecord()) {
            switch (subrecord.signature()) {
               case 'SNAM':
                  subrecord.read(uib.extra_form_ids[(word_count * 2) + 0]);
                  subrecord.read(uib.extra_form_ids[(word_count * 2) + 1]);
                  ++word_count;
                  break;
            }
         }
         return;
      }
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
               ++word_count;
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
      for (; word_count < 3; ++word_count) {
         uib.add_outbound_reference(uib.extra_form_ids[(word_count * 2) + 0]);
         uib.add_outbound_reference(uib.extra_form_ids[(word_count * 2) + 1]);
      }
   }
   void Shout::setup(const file_load_order& load_order) noexcept {
   }
   bool Shout::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Shout*)out;
      //
      // NOTE: Form::clone already took care of the form flags, including the "treat as power" flag.
      copy->name        = this->name;
      copy->description = this->description;
      copy->menu_display_object.set(*copy, this->menu_display_object);
      //
      for (size_t i = 0; i < this->words.size(); ++i) {
         auto& word = copy->words[i];
         auto& from = this->words[i];
         word.word_of_power.set(*copy, from.word_of_power);
         word.spell.set(*copy, from.spell);
         word.recoveryTime = from.recoveryTime;
      }
      return true;
   }
   bool Shout::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
      this->equip_type.clear_if(*this, other);
      this->menu_display_object.clear_if(*this, other);
      for (auto& word : this->words) {
         word.word_of_power.clear_if(*this, other);
         word.spell.clear_if(*this, other);
      }
   }
   void Shout::_clear_impl() noexcept {
      this->name.reset();
      this->description.reset();
      this->equip_type.set(*this, nullptr);
      this->menu_display_object.set(*this, nullptr);
      for (auto& word : this->words) {
         word.word_of_power.set(*this, nullptr);
         word.spell.set(*this, nullptr);
         word.recoveryTime = 0.0F;
      }
   }
}