#include "./shout.h"
#include "dovah/form_data/helpers/subrecord_was_handled_at_stub_creation.h"
#include "dovah/load_order_interfaces/form_load.h"
#include "dovah/load_order_processes/use_info_build/use_info_builder.h"
#include "dovah/tes/reading/record_view.h"
#include "dovah/tes/reading/subrecord_view.h"
#include "dovah/types/form_id.h"

#include "dovah/notices/form_load_warnings/by_form_type/shout/wrong_word_count.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::shout;
   }
}

namespace dovah::form_data_function_tables::build_outbound_use_info {
   void entry<form_type::shout>::execute(tes::record_view& record, load_order_processes::use_info_build::use_info_builder& uib) {
      using loaded_form_data = form_data::shout<managed_form_params>;
      constexpr const size_t max_word_count = std::tuple_size_v<decltype(loaded_form_data::words)>;

      static_assert(
         std::tuple_size_v<std::decay_t<decltype(uib.extra_form_ids)>> >= (max_word_count * 2),
         "UIBs must offer enough room for enough \"extra form IDs\" to accommodate the two forms per word in a Shout."
      );

      size_t word_count = 0;
         
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
                  if (word_count >= max_word_count)
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
      form_id equip_type;
      form_id menu_display_object;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'MDOB': // menu display object
               subrecord.read(menu_display_object);
               break;
            case 'ETYP': // equip type
               subrecord.read(equip_type);
               break;
            case 'SNAM':
               if (word_count >= max_word_count)
                  break;
               {
                  form_id id;
                  if (subrecord.read(id)) {
                     uib.add_outbound_use(id);
                     if (subrecord.read(id)) {
                        uib.add_outbound_use(id);
                        // and then a four-byte float, which we can ignore
                     }
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
      uib.add_outbound_use(equip_type);
      uib.add_outbound_use(menu_display_object);
      //
      // If the winning record defines fewer than three words (i.e. fewer than three 
      // SNAM subrecords), then we want the SNAMs from the losing records to "bleed 
      // through." We stored those in `uib.extra_form_ids` when handling the losing 
      // records earlier, so let's now pull from those for all words after the ones 
      // we've loaded during this go-around.
      //
      for (size_t i = word_count; i < max_word_count; ++i) {
         uib.add_outbound_use(uib.extra_form_ids[(i * 2) + 0]);
         uib.add_outbound_use(uib.extra_form_ids[(i * 2) + 1]);
      }
   }
}
namespace dovah::form_data_function_tables::serialize_form_data {
   extern void load(form_data::shout<managed_form_params>& self, tes::record_view& record, load_order_interfaces::form_load& intfc) {
      size_t current_word = 0;
      auto   _load_current_word = [&self, &intfc, &current_word](tes::subrecord_view& subrecord) {
         if (current_word < self.words.size()) {
            auto& entry = self.words[current_word];
            subrecord.read(entry.word_of_power);
            subrecord.read(entry.spell);
            subrecord.read(entry.recovery_time);
            intfc.warn_if_use_is_wrong_type(entry.word_of_power, form_type::word_of_power, subrecord, current_word);
            intfc.warn_if_use_is_wrong_type(entry.spell,         form_type::spell,         subrecord, current_word);
         }
         ++current_word;
      };

      if (intfc.is_winning_record) {
         while (auto& subrecord = record.next_subrecord()) {
            if (form_data_helpers::subrecord_was_handled_at_stub_creation(subrecord.signature()))
               continue;
            switch (subrecord.signature()) {
               case 'FULL':
                  subrecord.read(self.name);
                  break;
               case 'MDOB':
                  subrecord.read(self.menu_display_object);
                  intfc.warn_if_use_is_wrong_type(self.menu_display_object, form_type::statik, subrecord);
                  break;
               case 'ETYP':
                  subrecord.read(self.equip_type);
                  intfc.warn_if_use_is_wrong_type(self.equip_type, form_type::equip_slot, subrecord);
                  break;
               case 'DESC':
                  subrecord.read(self.description);
                  break;
               case 'SNAM':
                  _load_current_word(subrecord);
                  break;
               default:
                  intfc.warn_on_unrecognized_subrecord(subrecord);
                  break;
            }
         }
      } else {
         //
         // The words of a shout don't get reset in ClearData or InitializeData, presumably 
         // because Bethesda expects  there to always be exactly  three of them. This means 
         // that an override with fewer than three  words will cause the latter words of an 
         // overridden record to "leak through."
         //
         while (auto& subrecord = record.next_subrecord()) {
            if (form_data_helpers::subrecord_was_handled_at_stub_creation(subrecord.signature()))
               continue;
            switch (subrecord.signature()) {
               case 'SNAM':
                  _load_current_word(subrecord);
                  break;
               default:
                  intfc.warn_on_unrecognized_subrecord(subrecord);
                  break;
            }
         }
      }

      if (current_word != self.words.size()) {
         //
         // The game always assumes that SHOU will have three SNAMs. The Rule of One will not be properly 
         // applied if a SHOU override supplies fewer than three SNAMs.
         //
         intfc.log_load_warning(specific_load_warnings::wrong_word_count(
            self.stub,
            current_word
         ));
      }
   }
}