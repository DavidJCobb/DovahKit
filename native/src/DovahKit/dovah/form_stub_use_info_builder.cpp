#include "form_stub_use_info_builder.h"

namespace dovah {
   form_stub_use_info_builder::form_stub_use_info_builder(form_stub& s) : _stub(s) {
   }
   void form_stub_use_info_builder::add_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags) {
      if (++this->_pending.size < preallocated_array_size) {
         this->_pending.fixed[this->_pending.size] = _pending_entry(to_stub, flags);
      } else {
         this->_pending.extra.emplace_back(to_stub, flags);
      }
   }
   void form_stub_use_info_builder::add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags) {
      if (++this->_pending.size < preallocated_array_size) {
         this->_pending.fixed[this->_pending.size] = _pending_entry(toFormID, flags);
      } else {
         this->_pending.extra.emplace_back(toFormID, flags);
      }
   }
   void form_stub_use_info_builder::commit() {
      size_t size = this->_pending.size;
      if (size > preallocated_array_size)
         size = preallocated_array_size;
      for (size_t i = 0; i < size; ++i) {
         auto& entry = this->_pending.fixed[i];
         if (auto* to_stub = entry.target_stub)
            this->_stub._add_one_way_outbound_reference(to_stub, entry.flags);
         else
            this->_stub._add_one_way_outbound_reference(entry.target_id, entry.flags);
      }
      for (auto& entry : this->_pending.extra) {
         if (auto* to_stub = entry.target_stub)
            this->_stub._add_one_way_outbound_reference(to_stub, entry.flags);
         else
            this->_stub._add_one_way_outbound_reference(entry.target_id, entry.flags);
      }
      this->_pending.size = 0;
      this->_pending.extra.clear();
   }
   //
   form_stub_use_info_builder* form_stub_use_info_builder::spawn_subordinate() const noexcept {
      auto sub = new form_stub_use_info_builder(this->_stub);
      sub->_is_final_file     = this->_is_final_file;
      sub->_last_record_flags = this->_last_record_flags;
      sub->is_partial_record  = this->is_partial_record;
      return sub;
   }
   //
   void form_stub_use_info_builder::clear_all_prior_use_info() const noexcept {
      use_info_entry parent_entry;
      //
      auto& stub = this->_stub;
      for (auto& pair : stub.outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_child_of)
            parent_entry = entry;
      }
      stub.outbound.clear();
      if (auto* parent = parent_entry.other) {
         //
         // Do not clear parent/child relationships.
         //
         parent_entry.refcount = 1;
         stub.outbound[parent->formID] = parent_entry;
      }
   }
}