#include "Shout.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/shout/wrong_word_count.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::shout;
   }
}

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
               subrecord.read(this->name);
               break;
            case 'MDOB':
               subrecord.read(this->menu_display_object);
               intfc.warn_if_ref_is_wrong_type(this->menu_display_object, form_type::statik, subrecord.signature());
               break;
            case 'ETYP':
               subrecord.read(this->equip_type);
               intfc.warn_if_ref_is_wrong_type(this->equip_type, form_type::equip_slot, subrecord.signature());
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'SNAM':
               if (current_word < this->words.size()) {
                  auto& entry = this->words[current_word];
                  subrecord.read(entry.word_of_power);
                  subrecord.read(entry.spell);
                  subrecord.read(entry.recoveryTime);
                  
                  intfc.warn_if_ref_is_wrong_type(entry.word_of_power, form_type::word_of_power, subrecord, { .nth_reference = current_word });
                  intfc.warn_if_ref_is_wrong_type(entry.spell,         form_type::spell,         subrecord, { .nth_reference = current_word });
               }
               ++current_word;
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (current_word != this->words.size()) {
         //
         // The game always assumes that SHOU will have three SNAMs. The Rule of One will not be properly 
         // applied if a SHOU override supplies fewer than three SNAMs.
         //
         specific_load_warnings::wrong_word_count notice(
            this->stub,
            current_word
         );
         intfc.log_load_warning(notice);
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
                  if (word_count >= std::tuple_size_v<decltype(words)>)
                     break;
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
      form_id_t equip_type;
      form_id_t menu_display_object;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'MDOB': // menu display object
               subrecord.read(menu_display_object);
               break;
            case 'ETYP': // equip type
               subrecord.read(equip_type);
               break;
            case 'SNAM':
               if (word_count >= std::tuple_size_v<decltype(words)>)
                  break;
               if (subrecord.read(formID)) {
                  uib.add_outbound_reference(formID);
                  if (subrecord.read(formID)) {
                     uib.add_outbound_reference(formID);
                     // and then a four-byte float, which we can ignore
                  }
               }
               ++word_count;
               break;
            case 'EDID': // editor ID
            case 'FULL': // displayed name
            case 'DESC': // description
               break;
         }
      }
      uib.add_outbound_reference(equip_type);
      uib.add_outbound_reference(menu_display_object);
      for (; word_count < 3; ++word_count) {
         uib.add_outbound_reference(uib.extra_form_ids[(word_count * 2) + 0]);
         uib.add_outbound_reference(uib.extra_form_ids[(word_count * 2) + 1]);
      }
   }
   void Shout::setup(const file_load_order& load_order) noexcept {
   }
   void Shout::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Shout*)out;
      
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
   }
   void Shout::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      record.write_formID_subrecord('MDOB', this->menu_display_object, true);
      record.write_formID_subrecord('ETYP', this->equip_type, true);
      auto& DESC = record.open_next_subrecord('DESC');
      DESC.write(this->description);
      DESC.close();
      for (auto& word : this->words) {
         auto& SNAM = record.open_next_subrecord('SNAM');
         SNAM.reserve_more(0xC);
         SNAM.write(word.word_of_power);
         SNAM.write(word.spell);
         SNAM.write(word.recoveryTime);
         SNAM.close();
      }
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