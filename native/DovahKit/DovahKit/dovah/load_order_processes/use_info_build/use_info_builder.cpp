#include "./use_info_builder.h"
#include "dovah/_passkeys/build_use_info_during_load.h"
#include "dovah/form_stub.h"
#include "dovah/use_info/entry.h"
#include "dovah/use_info/entry_flags/base.h"
#include "./use_info_builder_form_specific_data.h"

namespace dovah::load_order_processes::use_info_build {
   use_info_builder::use_info_builder(form_stub& s) : _stub(s) {
   }
   use_info_builder::~use_info_builder() {
      if (auto*& p = this->_form_specific_data) {
         delete p;
         p = nullptr;
      }
   }

   void use_info_builder::add_outbound_use(form_id used_form_id, use_info::entry_flag_underlying_type flags) {
      if (!used_form_id)
         return;
      this->_pending.push_back(pending_entry{
         .target_id = (bare_form_id)used_form_id,
         .flags     = flags
      });
   }
   void use_info_builder::cancel_outbound_use(form_id used_form_id, use_info::entry_flag_underlying_type flags) {
      const auto   bare_used_form_id = (bare_form_id)used_form_id;
      const size_t size              = this->_pending.size();
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->_pending[i];
         if (entry.target_id == bare_used_form_id && entry.flags == flags) {
            this->_pending.erase(this->_pending.begin() + i);
            return;
         }
      }
   }
   void use_info_builder::commit() {
      if (this->_parent) {
         if (auto* fsd = this->_form_specific_data) {
            fsd->commit(*this->_parent);
            fsd->clear();
         }
         for (auto& entry : this->_pending) {
            this->_parent->_pending.push_back(entry);
         }
         this->_pending.clear();
         return;
      }

      if (auto* fsd = this->_form_specific_data) {
         fsd->commit(*this);
         fsd->clear();
      }
      for (auto& entry : this->_pending) {
         if (auto* to_stub = entry.target_stub)
            this->_stub._add_one_way_outbound_use({}, *to_stub, entry.flags);
         else
            this->_stub._add_one_way_outbound_use({}, entry.target_id, entry.flags);
      }
      this->_pending.clear();
   }
   
   [[nodiscard]] use_info_builder* use_info_builder::spawn_subordinate() noexcept {
      auto sub = new use_info_builder(this->_stub);
      sub->_parent            = this;
      sub->_is_active_file    = this->_is_active_file;
      sub->_is_base_record    = this->_is_base_record;
      sub->_is_final_file     = this->_is_final_file;
      sub->_is_partial_record = this->_is_partial_record;
      return sub;
   }
   [[nodiscard]] use_info_builder use_info_builder::spawn_subordinate_on_stack() noexcept {
      auto sub = use_info_builder(this->_stub);
      sub._parent             = this;
      sub._is_active_file     = this->_is_active_file;
      sub._is_base_record     = this->_is_base_record;
      sub._is_final_file      = this->_is_final_file;
      sub._is_partial_record  = this->_is_partial_record;
      return sub;
   }
   
   void use_info_builder::clear_pending_use_info() noexcept {
      if (auto* fsd = this->_form_specific_data) {
         fsd->clear();
      }
      this->_pending.clear();
   }
   void use_info_builder::clear_all_prior_use_info() const noexcept {
      use_info::entry parent_entry;
      //
      auto& stub = this->_stub;
      for (auto& pair : stub.outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent))
            parent_entry = entry;
      }
      stub.outbound.clear();
      if (auto* parent = parent_entry.other) {
         //
         // Do not clear parent/child relationships.
         //
         parent_entry.refcount = 1;
         stub.outbound[parent->form_id] = parent_entry;
      }
   }

   use_info_builder::form_specific_data* use_info_builder::get_form_specific_data() {
      if (auto* p = this->_form_specific_data)
         return p;
      this->_form_specific_data = new form_specific_data;
      return this->_form_specific_data;
   }
}