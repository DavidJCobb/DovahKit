#include "./package_data_declaration_map.h"
#include <bitset>
#include "../../_common_cpp.h"

#include "../custom_packages/package_data.h"

#include "../../../notices/form_load_warnings/by_form_type/package/package_data_metadata_belongs_to_none.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unique_id_has_multiple_metadata.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   void package_data_declaration_map::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      if (record.get_current_subrecord().signature() != subrecord_unique_id)
         return;

      std::bitset<255> used_ids;
      size_t which = 0;
      do {
         auto& entry = this->entries.emplace_back();
         record.get_current_subrecord().read(entry.unique_id);
         record.next_subrecord();

         if (entry.unique_id == package_data::no_unique_id) {
            specific_load_warnings::package_data_metadata_belongs_to_none notice(
               intfc.target_stub,
               which
            );
            intfc.log_load_warning(notice);
         } else {
            if (used_ids.test(entry.unique_id)) {
               specific_load_warnings::unique_id_has_multiple_metadata notice(
                  intfc.target_stub,
                  entry.unique_id
               );
               intfc.log_load_warning(notice);
            }
            used_ids.set(entry.unique_id);
         }

         {  // Handle BNAM if present.
            auto& subrecord = record.get_current_subrecord();
            if (subrecord.signature() == subrecord_name) {
               subrecord.read(entry.name);
               record.next_subrecord();
            }
         }
         {  // Handle PNAM if present.
            auto& subrecord = record.get_current_subrecord();
            if (subrecord.signature() == subrecord_access) {
               {
                  uint32_t dword;
                  subrecord.read(dword);
                  entry.is_public = dword == 1;
               }
               record.next_subrecord();
            }
         }
         //
         // Done with this packdata.
         //
      } while (++which, record.get_current_subrecord().signature() == subrecord_unique_id);
   }
   /*static*/ void package_data_declaration_map::generate_use_info(tes_record_reader& record, form_stub_use_info_builder&) {
      //
      // We don't actually contain any uses, but we need to skip the right number 
      // of subrecords.
      //
      if (record.get_current_subrecord().signature() != subrecord_unique_id)
         return;
      do {
         record.next_subrecord();
         if (record.get_current_subrecord().signature() == subrecord_name)
            record.next_subrecord();
         if (record.get_current_subrecord().signature() == subrecord_access)
            record.next_subrecord();
      } while (record.get_current_subrecord().signature() == subrecord_unique_id);
   }
   void package_data_declaration_map::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      for (auto& entry : this->entries) {
         {
            auto& subrecord = record.open_next_subrecord(subrecord_unique_id);
            subrecord.write(entry.unique_id);
            subrecord.close();
         }
         record.write_string_subrecord(subrecord_name, entry.name);
         {
            auto& subrecord = record.open_next_subrecord(subrecord_access);
            uint32_t data = entry.is_public ? 1 : 0;
            subrecord.write(data);
            subrecord.close();
         }
      }
   }
}