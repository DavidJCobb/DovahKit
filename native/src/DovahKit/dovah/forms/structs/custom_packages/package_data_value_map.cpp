#include "./package_data_value_map.h"
#include <bitset>
#include "../../_common_cpp.h"

#include "../custom_packages/package_data.h"

#include "../../../notices/form_load_warnings/by_form_type/package/package_data_wants_none_as_unique_id.h"
#include "../../../notices/form_load_warnings/by_form_type/package/too_many_package_data.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unexpected_subrecord_in_unique_id_list.h"
#include "../../../notices/form_load_warnings/by_form_type/package/unique_id_used_by_multiple_package_data.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace dovah::loaded_forms::structs::custom_packages {
   void package_data_value_map::load(tes_record_reader& record, load_order_interfaces::form_load& intfc, size_t count) {
      if (count > 255) {
         specific_load_warnings::too_many_package_data notice(
            intfc.target_stub,
            count
         );
         intfc.log_load_warning(notice);
      }
      //
      // Spawn [our equivalent to] BGSPackageData objects and load their values.
      //
      for (size_t i = 0; i < count; ++i) {
         auto& subrecord = record.next_subrecord();
         auto& entry     = this->entries.emplace_back();

         custom_packages::package_data::load_context context;
         context.which = this->entries.size();

         static_assert(false, "TODO: this should return a unique_ptr, and should not be allowed to return null.");
         auto* packdata = custom_packages::package_data::load_content(record, intfc, context);
         assert(packdata != nullptr);
         entry.value.reset(packdata);
      }
      //
      // Load the mapping of BGSPackageData indices to unique IDs.
      //
      std::bitset<255> ids_already_assigned;
      for (uint32_t i = 0; i < count; ++i) {
         auto& subrecord = record.next_subrecord();
         auto& entry     = this->entries[i];
         auto* packdata  = entry.value.get();
         switch (subrecord.signature()) {
            case 'UNAM':
               {
                  uint8_t unique_id = package_data::no_unique_id;
                  if (subrecord.read(unique_id)) {
                     entry.unique_id = unique_id;
                     if (unique_id == package_data::no_unique_id) {
                        specific_load_warnings::package_data_wants_none_as_unique_id notice(
                           intfc.target_stub,
                           i,
                           packdata ? packdata->get_type() : dovah::packages::package_data_type::invalid
                        );
                        intfc.log_load_warning(notice);
                     }
                  }
                  if (unique_id < ids_already_assigned.size()) {
                     if (ids_already_assigned.test(unique_id)) {
                        specific_load_warnings::unique_id_used_by_multiple_package_data notice(
                           intfc.target_stub,
                           unique_id
                        );
                        intfc.log_load_warning(notice);
                     }
                     ids_already_assigned.set(unique_id);
                  }
               }
               break;
            default:
               {
                  specific_load_warnings::unexpected_subrecord_in_unique_id_list notice(
                     intfc.target_stub,
                     i,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               this->next_unique_id = i + 1;
               entry.unique_id = i;
               if (i == package_data::no_unique_id) {
                  specific_load_warnings::package_data_wants_none_as_unique_id notice(
                     intfc.target_stub,
                     i,
                     packdata ? packdata->get_type() : dovah::packages::package_data_type::invalid
                  );
                  intfc.log_load_warning(notice);
               }
               break;
         }
      }

      record.next_subrecord();

      //
      // Load next unique ID.
      //
      {
         auto& subrecord = record.get_current_subrecord();
         if (subrecord.signature() == subrecord_next_uid) {
            subrecord.read(this->next_unique_id);
            record.next_subrecord();
         }
      }
   }
   /*static*/ void package_data_value_map::generate_use_info(tes_record_reader& record, size_t count, std::vector<form_id_t>& out) {
      // Value list:
      for (size_t i = 0; i < count; ++i) {
         auto& subrecord = record.next_subrecord();
         custom_packages::package_data::generate_use_info(record, out);
      }

      // Unique ID list:
      for (uint32_t i = 0; i < count; ++i) {
         auto& subrecord = record.next_subrecord();
      }

      // Next unique ID:
      record.next_subrecord();
      if (record.get_current_subrecord().signature() == subrecord_next_uid)
         record.next_subrecord();
   }
   void package_data_value_map::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      for (auto& entry : this->entries) {
         assert(entry.value != nullptr);
         entry.value->save_typename(record, intfc);
         entry.value->save_value(record, intfc);
      }
      for (auto& entry : this->entries) {
         auto& subrecord = record.open_next_subrecord(subrecord_unique_id);
         subrecord.write(entry.unique_id);
         subrecord.close();
      }

      {
         auto& subrecord = record.open_next_subrecord(subrecord_next_uid);
         subrecord.write(this->next_unique_id);
         subrecord.close();
      }
   }
   void package_data_value_map::clone_from(const package_data_value_map& src, Form& my_owner) noexcept {
      this->clear(my_owner);

      for (auto& src_entry : src.entries) {
         auto& dst_entry = this->entries.emplace_back();
         dst_entry.unique_id = src_entry.unique_id;
         if (src_entry.value) {
            dst_entry.value.reset(src_entry.value->clone(my_owner));
         }
      }
      this->next_unique_id = src.next_unique_id;
   }
   void package_data_value_map::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept {
      for (auto& entry : this->entries) {
         auto* packdata = entry.value.get();
         if (packdata)
            packdata->sever_outbound_references_to(other, my_owner);
      }
   }
   void package_data_value_map::clear(loaded_forms::Form& my_owner) {
      for (auto& entry : this->entries) {
         auto* packdata = entry.value.get();
         if (packdata)
            packdata->clear(my_owner);
      }
      this->entries.clear();
   }
}