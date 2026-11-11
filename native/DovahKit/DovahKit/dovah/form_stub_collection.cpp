#include "./form_stub_collection.h"
#include <cassert>
#include "./_impl/form_stub_collection/create_hardcoded_form_stubs.h"
#include "./_impl/form_stub_collection/create_none_stubs.h"
#include "./active_load_order.h"
#include "./exceptions/game_setting_edit_failed.h"
#include "./form_data/function_tables/clear_outbound_uses.h"
#include "./form_stub.h"
#include "./form_type_info.h"
#include "./requests/create_form.h"
#include "./tes/file.h"
#include "./types/file_prefix.h"
#include "./utils/form_stubs/can_load_form_data.h"
#include "./utils/form_stubs/is_defined_in_active_file.h"
#include "./utils/form_stubs/is_exterior_cell.h"

namespace dovah {
   bare_form_id form_stub_collection::find_first_free_form_id_in_active_file(bare_form_id search_from) const noexcept {
      file_prefix active_prefix = this->owner.expected_active_file_prefix_post_save();
      if (active_prefix.is_undefined())
         return 0;
      bare_form_id min_id = active_prefix.min_form_id();
      bare_form_id max_id = active_prefix.max_form_id();
      bare_form_id id = search_from ? active_prefix.coerce_form_id(search_from) : min_id;
      //
      auto& map = this->active_file_forms;
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
      auto&      map     = this->forms_by_type[ft];
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
   form_stub& form_stub_collection::get_or_create_canonical_instance_of_singleton_form(form_type ft) {
      if (auto* stub = this->get_canonical_instance_of_singleton_form(ft))
         return *stub;

      requests::create_form request{ this->owner };
      request.form_type = ft;
      return request.commit();
   }

   void form_stub_collection::_add_hardcoded_form(form_stub& stub) {
      const bare_form_id formID = stub.form_id;
      auto& type = this->forms_by_type[stub.form_type];

      std::lock_guard<std::mutex> guard(this->forms_lock);
      type[formID] = &stub;
      this->forms[formID] = &stub;
   }
   bare_form_id form_stub_collection::_reserve_form_id() {
      auto& alo         = this->owner;
      auto* active_file = alo.get_active_file();
      if (!active_file)
         return 0;
      auto prefix  = alo.expected_active_file_prefix_post_save();
      auto form_id = prefix.coerce_form_id(active_file->header().nextFormID);
         
      auto  guard = std::lock_guard(this->_reserved_form_ids.lock);
      auto& list  = this->_reserved_form_ids.list;
      {
         auto guard = std::lock_guard(this->forms_lock);
         if (std::find(list.begin(), list.end(), form_id) != list.end()) // if the form ID is reserved, then we need to find a new one
            form_id = 0;
         if (!(form_id & 0x00FFFFFF) || this->get_form_by_id(form_id, true) != nullptr) {
            form_id = this->find_first_free_form_id_in_active_file();
         }
         if (this->prefs.new_forms_avoid_extended_esl_form_id_range && form_id) {
            auto local_id = form_id & 0x00FFFFFF;
            if (local_id < 0x800) {
               auto alternative_id = this->find_first_free_form_id_in_active_file(0x00000800);
               if (alternative_id)
                  form_id = alternative_id;
            }
         }
      }
      if (form_id)
         list.push_back(form_id);
      return form_id;
   }

   //
   // passkeyed
   //

   void form_stub_collection::_destroy_none_stub(lu::passkey<requests::create_form>, form_stub& none_stub) {
      assert(none_stub.is_none_stub());
      assert(none_stub.outbound.empty()); // a none-stub should never refer to other forms
      
      std::vector<form_stub*> pending;
      
      for (auto& pair : none_stub.inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         auto  form = other->load();
         pending.push_back(other); // gather forms to process later. we don't want to sever refs now, as that will change use info and potentially invalidate iterators during the loop
         other->set_edited(true);
      }
      for (auto* user : pending) {
         auto form    = user->load();
         auto handler = form_data::function_tables::clear_outbound_uses::lookup(user->form_type);
         assert(!!handler);
         (handler)(*form);
      }
      //
      if (auto callback = this->owner.post_load_callbacks.on_form_lost)
         (callback)(none_stub);
      //
      const bare_form_id form_id = none_stub.form_id;
      this->forms.erase(form_id);
      this->forms_by_type[form_type::none].erase(form_id);
      this->active_file_forms.erase(form_id);
      this->active_file_forms_by_type[form_type::none].erase(form_id);
      //
      delete &none_stub;
   }
   void form_stub_collection::_make_hardcoded_forms(lu::passkey<active_load_order>, tes::file& hardcoded_forms_file) {
      impl::_form_stub_collection::create_hardcoded_form_stubs(*this, hardcoded_forms_file);
   }
   void form_stub_collection::_make_none_stubs(lu::passkey<active_load_order>, tes::file& none_stubs_file) {
      impl::_form_stub_collection::create_none_stubs(*this, none_stubs_file);
   }
   void form_stub_collection::_on_stub_flagged_as_edited(lu::passkey<form_stub>, form_stub& stub) {
      this->active_file_forms[stub.form_id] = &stub;
      auto& at = this->active_file_forms_by_type[stub.form_type];
      at[stub.form_id] = &stub;
      //
      // NOTE: Merely flagging a form stub as edited SHOULD NOT result in the active file being 
      // added to the stub's source file list (if not already present). Currently, our handling 
      // of singleton forms depends on no such addition being made (refer to documentation for 
      // more information).
      //
   }
   void form_stub_collection::_reserve(lu::passkey<active_load_order>, size_t expected_total_form_count) {
      this->forms.reserve((size_t)(expected_total_form_count * 1.1F) + 0x800);
   }
   void form_stub_collection::_store_newly_created_form(lu::passkey<requests::create_form>, form_stub& stub) {
      auto guard = std::lock_guard(this->forms_lock);
      this->forms[stub.form_id] = &stub;
      this->forms_by_type[stub.form_type][stub.form_id] = &stub;
      this->active_file_forms[stub.form_id] = &stub;
      this->active_file_forms_by_type[stub.form_type][stub.form_id] = &stub;
   }

   bare_form_id form_stub_collection::_reserve_form_id(lu::passkey<requests::create_form>) {
      return this->_reserve_form_id();
   }
   void form_stub_collection::_abandon_form_id_reservation(lu::passkey<requests::create_form>, bare_form_id id) {
      if (!id)
         return;
      auto  guard = std::lock_guard(this->_reserved_form_ids.lock);
      auto& list  = this->_reserved_form_ids.list;
      auto  it    = std::find(list.begin(), list.end(), id);
      if (it != list.end()) {
         list.erase(it);
      }
   }

   form_stub& form_stub_collection::_make_stub_for_game_setting(lu::passkey<loaded_game_setting>, const std::string& editor_id) {
      tes::file* active_file = this->owner.get_active_file();
      if (!active_file) {
         throw exceptions::game_setting_edit_failed(
            exceptions::game_setting_edit_failed::error_code::no_active_file,
            editor_id
         );
      }
      bare_form_id form_id = this->_reserve_form_id();
      if (!form_id) {
         throw exceptions::game_setting_edit_failed(
            exceptions::game_setting_edit_failed::error_code::no_form_id_available,
            editor_id
         );
      }

      auto* stub = new form_stub;
      stub->editor_id = editor_id;
      stub->form_id   = form_id;
      stub->form_type = form_type::setting;
      stub->_add_source_file({}, *active_file, 0);
      stub->set_edited(true);

      auto guard = std::lock_guard(this->forms_lock);
      this->forms[form_id] = stub;
      this->forms_by_type[form_type::setting][form_id] = stub;
      this->active_file_forms[form_id] = stub;
      this->active_file_forms_by_type[form_type::setting][form_id] = stub;

      return *stub;
   }
}