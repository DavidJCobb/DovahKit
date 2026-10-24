#include "./form_stub_collection.h"
#include "./active_load_order.h"
#include "./form_stub.h"
#include "./form_type_info.h"
#include "./_impl/form_stub_collection/create_hardcoded_form_stubs.h"
#include "./utils/form_stubs/can_load_form_data.h"
#include "./utils/form_stubs/is_defined_in_active_file.h"
#include "./utils/form_stubs/is_exterior_cell.h"
#include "./types/file_prefix.h"

#include "dovah/load_order_processes/build_form_stubs_from_file/check_and_report_override_legality.h"
#include "dovah/load_order_processes/build_form_stubs_from_file/fold_overriding_stub_into_overridden_stub.h"
#include "dovah/load_order_processes/build_form_stubs_from_file/set_file_list_for_newest_singleton_stub.h"
#include "dovah/load_order_processes/build_form_stubs_from_file/update_stub_addenda_on_commit.h"
#include "dovah/notices/file_parse_warnings/esl_defined_an_interior_cell.h"
#include "dovah/notices/file_parse_warnings/form_initial_record_is_partial.h"
#include "dovah/notices/file_parse_warnings/the_game_doesnt_load_new_actor_value_infos.h"
#include "dovah/utils/form_stubs/get_parent_form.h"
#include "dovah/utils/form_stubs/is_interior_cell.h"

namespace dovah {
   bare_form_id form_stub_collection::find_first_free_form_id_in_active_file(bare_form_id search_from) const noexcept {
      file_prefix active_prefix = this->owner.expected_active_file_prefix_post_save();
      if (active_prefix.is_undefined())
         return 0;
      bare_form_id min_id = active_prefix.min_form_id();
      bare_form_id max_id = active_prefix.max_form_id();
      bare_form_id id = search_from ? active_prefix.coerce_form_id(id) : min_id;
      //
      auto& map = this->active_file_forms.forms;
      if (map.size() >= (max_id - min_id)) // no form IDs available
         return 0;
      //
      auto& reservations = this->_reserved_form_ids.list;
      auto  guard        = std::lock_guard(this->_reserved_form_ids.lock);
      //
      for (; id < max_id; ++id) {
         form_stub* stub = nullptr;
         {
            auto it = map.find(id);
            if (it != map.end())
               stub = it->second;
         }
         if (stub && !stub->is_none_stub())
            continue;
         if (std::find(reservations.begin(), reservations.end(), id) != reservations.end())
            continue;
         if (stub && stub->is_none_stub()) {
            //
            // If we're capable of destroying this none-stub, then treat its form ID as available.
            // 
            // None-stubs can be destroyed if they come solely from the active file, and if all 
            // of the forms that reference them can be loaded.
            //
            bool available = true;
            for (auto& pair : stub->inbound) {
               auto& entry = pair.second;
               if (!entry.other)
                  continue;
               form_stub& other = *entry.other;
               if (!utils::form_stubs::is_defined_in_active_file(other)) {
                  available = false;
                  break;
               }
               if (!utils::form_stubs::can_load_form_data(other)) {
                  available = false;
                  break;
               }
            }
            if (!available)
               continue;
         }
         return id;
      }
      return 0;
   }

   form_stub* form_stub_collection::get_canonical_instance_of_singleton_form(form_type ft) const noexcept {
      if (!(form_type_info::lookup(ft).flags & form_type_info::flag::is_singleton))
         return nullptr;
      uint16_t   length  = 0;
      form_stub* longest = nullptr;
      auto&      map     = this->forms_by_type[ft].forms;
      for (auto pair : map) {
         auto* stub = pair.second;
         if (!stub)
            continue;
         const auto count = stub->source_file_count();
         if (count > length) {
            length  = count;
            longest = stub;
         }
      }
      return longest;
   }
   //form_stub* form_stub_collection::get_or_create_canonical_instance_of_singleton_form(form_type); // TODO

   void form_stub_collection::_add_hardcoded_form(form_stub& stub) {
      auto& type = this->forms_by_type[stub.form_type];
      std::lock_guard<std::mutex> guard_for_form_type(type.lock);
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);
      
      const bare_form_id formID = stub.form_id;
      type.forms[formID] = &stub;
      this->forms.forms[formID] = &stub;
   }

   //
   // passkeyed
   //

   void form_stub_collection::_take_newly_parsed_form_stub(
      const callback_sets::data_load& callbacks,
      const tes::file& source_file,
      const bool source_file_is_active,
      size_t record_offset,
      const tes::record_header& record_header,
      form_stub*& stub
   ) {
      std::lock_guard<std::mutex> guard_for_all_forms(this->forms.lock);

      const bare_form_id record_id  = record_header.formID;
      const bare_form_id form_id    = stub->form_id;
      form_stub*         new_parent = utils::form_stubs::get_parent_form(*stub); // set by caller before calling us

      form_stub* existing_form = nullptr;
      {
         auto it = this->forms.forms.find(form_id);
         if (it != this->forms.forms.end())
            existing_form = it->second;
      }

      if (existing_form) { // is this an override?
         load_order_processes::build_form_stubs_from_file::check_and_report_override_legality(
            callbacks,
            *stub,
            *existing_form,
            record_offset,
            record_id
         );
         load_order_processes::build_form_stubs_from_file::fold_overriding_stub_into_overridden_stub(
            callbacks,
            source_file,
            source_file_is_active,
            *stub,
            *existing_form,
            record_offset,
            record_id
         );
         //
         // Delete the new stub, and overwrite the pointer (in this function and in our caller(s)) with 
         // the existing stub.
         //
         delete stub;
         stub = existing_form;
      } else {
         //
         // This is not an override.
         //
         if (stub->form_type == form_type::actor_value_info) {
            if (callbacks.on_warning) {
               (callbacks.on_warning)(
                  notices::file_parse_warnings::the_game_doesnt_load_new_actor_value_infos(
                     source_file.path().filename().string(),
                     stub->form_id,
                     record_header,
                     record_offset
                  )
               );
            }
         }
         if (stub->source_file_count() == 1) {
            if (stub->test_record_flags((uint32_t)1 << (size_t)tes::record_flag::partial)) {
               bool injected = stub->is_injected();
               if (callbacks.on_warning) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::form_initial_record_is_partial(
                        source_file.path().filename().string(),
                        stub->form_id,
                        record_header,
                        record_offset,
                        injected
                     )
                  );
               }
               if (injected) {
                  //
                  // Skyrim will skip a partial record if it is injected and not an override. We'll 
                  // do the same. It's tempting to retain the form or to create a none-stub, but 
                  // either approach would incorrectly prevent subsequent files from injecting over 
                  // the same form ID.
                  //
                  delete stub;
                  stub = nullptr;
                  return;
               }
               //
               // Strip the flag for a non-injected non-override, as the game would.
               //
               stub->file.flags &= ~((uint32_t)1 << (size_t)tes::record_flag::partial);
            }
            if (callbacks.on_warning) {
               if (utils::form_stubs::is_interior_cell(*stub) && source_file.is_light()) {
                  (callbacks.on_warning)(
                     notices::file_parse_warnings::esl_defined_an_interior_cell(
                        source_file.path().filename().string(),
                        stub->form_id,
                        record_id
                     )
                  );
               }
            }
         }
         //
         // Insert the stub into the form maps:
         //
         auto& type = this->forms_by_type[stub->form_type];
         this->forms.forms[form_id] = stub;
         {
            std::lock_guard<std::mutex> guard_for_form_type(type.lock);
            type.forms[form_id] = stub;
         }
      }
      //
      // We've either stored the stub as new, or treated it as an override (moving relevant data to the 
      // existing stub, deleting the new stub, and overwriting `stub` with a pointer to the existing stub). 
      //
      // Place the stub inside of the active file form maps, if appropriate.
      //
      if (source_file_is_active) {
         this->active_file_forms.forms[form_id] = stub;
         this->active_file_forms_by_type[stub->form_type].forms[form_id] = stub;
      }
      //
      // Finally, some finishing touches.
      //
      load_order_processes::build_form_stubs_from_file::update_stub_addenda_on_commit(
         *stub,
         new_parent
      );
      if (form_type_info::lookup(stub->form_type).flags & form_type_info::flag::is_singleton) {
         load_order_processes::build_form_stubs_from_file::set_file_list_for_newest_singleton_stub(
            *this,
            callbacks,
            source_file,
            *stub,
            record_id
         );
      }
   }
   void form_stub_collection::_make_hardcoded_forms(lu::passkey<active_load_order>, tes::file& hardcoded_forms_file) {
      impl::_form_stub_collection::create_hardcoded_form_stubs(*this, hardcoded_forms_file);
   }
   void form_stub_collection::_make_none_stubs(lu::passkey<active_load_order>, tes::file& none_stubs_file) {
      impl::_form_stub_collection::create_none_stubs(*this, none_stubs_file);
   }
   void form_stub_collection::_on_stub_flagged_as_edited(lu::passkey<form_stub>, form_stub& stub) {
      this->active_file_forms.forms[stub.form_id] = &stub;
      auto& at = this->active_file_forms_by_type[stub.form_type];
      at.forms[stub.form_id] = &stub;
      //
      // NOTE: Merely flagging a form stub as edited SHOULD NOT result in the active file being 
      // added to the stub's source file list (if not already present). Currently, our handling 
      // of singleton forms depends on no such addition being made (refer to documentation for 
      // more information).
      //
   }
   void form_stub_collection::_reserve(lu::passkey<active_load_order>, size_t expected_total_form_count) {
      this->forms.forms.reserve((size_t)(expected_total_form_count * 1.1F) + 0x800);
   }
}