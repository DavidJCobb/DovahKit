#include "./form_deletion_request.h"
#include "../files/tes_file_reading/file_loader.h"
#include "../files/file_load_order.h"
#include "../forms/Form.h"
#include "../form_stub.h"
#include "../notice_code_list.h"

namespace dovah {
   form_deletion_request::form_deletion_request(file_load_order& o, form_stub& t) : owner(o), target(t) {
      if (this->target.is_hardcoded() || this->target.formID < minimum_plugin_form_id) {
         this->error = notice_code::cannot_delete_hardcoded_form;
         return;
      }
      this->active_file_prefix = this->owner.active_file_prefix();
      //
      if (_form_should_be_flagged(this->target)) {
         this->forms_needing_flag.insert(&this->target);
      } else {
         this->forms_needing_delete.insert(&this->target);
      }
      this->seen_stubs.insert(&this->target);
      this->_gather_others(&this->target);
   }
   form_deletion_request::form_deletion_request(form_deletion_request&& other) : owner(other.owner), target(other.target) {
      this->error = other.error;
      std::swap(this->forms_needing_delete, other.forms_needing_delete);
      std::swap(this->seen_stubs, other.seen_stubs);
      //
      this->force_delete_overrides = other.force_delete_overrides;
      //
      this->active_file_prefix = this->owner.active_file_prefix();
   }
   bool form_deletion_request::_form_should_be_flagged(form_stub& stub) {
      if (this->force_delete_overrides)
         return false;
      if (stub.is_hardcoded()) // we don't currently allow any kind of deletion of hardcoded forms, but it never hurts to be prepared for what might change
         return true;
      if (!this->active_file_prefix.contains_form_id(stub.formID))
         return true;
      return false;
   }
   void form_deletion_request::_gather_others(form_stub* start) {
      //
      // Recursively find all descendant forms that need to be deleted, and ensure that we can load 
      // not only all of the to-be-deleted forms, but also all of their users, in order to sever 
      // uses and set "deleted" flags as necessary.
      //
      if (this->error != default_notice_code)
         return;
      if (!start)
         start = &this->target;
      //
      if (!start->load()) {
         this->error = notice_code::unimplemented_form_type;
         return;
      }
      //
      for (auto& pair : start->inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         if (this->seen_stubs.find(other) != this->seen_stubs.end())
            continue;
         this->seen_stubs.insert(entry.other);
         //
         if (!entry.other->load()) { // Check to ensure we can load all users.
            this->error = notice_code::cannot_load_all_users_of_this_form;
            return;
         }
         //
         if (entry.flags & use_info_entry::flag::parent_child) {
            assert(!entry.other->is_hardcoded() && "How is a hardcoded form a descendant of a form that can be deleted (and in fact is currently being deleted)?");
            if (_form_should_be_flagged(*entry.other)) {
               this->forms_needing_flag.insert(entry.other);
            } else {
               this->forms_needing_delete.insert(entry.other);
            }
            this->_gather_others(entry.other);
            if (this->error != default_notice_code)
               return;
         }
      }
   }
   void form_deletion_request::_prep_for_delete(form_stub& stub, bool flag) {
      stub.sever_all_outbound_references();
      //
      std::vector<form_stub*> pending;
      //
      for (auto& pair : stub.inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         auto  form = other->load();
         pending.push_back(other); // gather forms to process later. we don't want to sever refs now, as that will change use info and potentially invalidate iterators during the loop
         if (!flag)
            other->sever_addenda_references_to(stub);
         other->set_edited(true);
      }
      for (auto* user : pending) {
         auto form = user->load();
         form->sever_outbound_references_to(stub);
      }
   }
   std::vector<form_stub*> form_deletion_request::get_forms_pending_delete(bool include_flagged) const noexcept {
      std::vector<form_stub*> out;
      if (include_flagged)
         out.reserve(this->forms_needing_delete.size() + this->forms_needing_flag.size());
      else
         out.reserve(this->forms_needing_delete.size());
      for (auto* stub : this->forms_needing_delete)
         out.push_back(stub);
      if (include_flagged)
         for (auto* stub : this->forms_needing_flag)
            out.push_back(stub);
      return out;
   }
   std::vector<form_stub*> form_deletion_request::get_forms_pending_flagging() const noexcept {
      std::vector<form_stub*> out;
      out.reserve(this->forms_needing_flag.size());
      for (auto* stub : this->forms_needing_flag)
         out.push_back(stub);
      return out;
   }
   void form_deletion_request::commit() {
      if (this->done || this->error != default_notice_code)
         return;
      if (!owner.active_file) {
         this->error = notice_code::no_active_file;
         return;
      }
      bare_form_id_t lowestID = 0xFFFFFFFF;
      for (auto* stub : this->forms_needing_delete) {
         this->_prep_for_delete(*stub, false);
         //
         auto formID = stub->formID;
         if (formID < lowestID)
            lowestID = formID;
         this->owner.forms.forms.erase(formID);
         this->owner.forms_by_type[stub->form_type].forms.erase(formID);
         this->owner.active_file_forms.forms.erase(formID);
         this->owner.active_file_forms_by_type[stub->form_type].forms.erase(formID);
         //
         delete stub;
      }
      this->forms_needing_delete.clear();
      if (auto* file = owner.active_file) {
         if (file->header.nextFormID > lowestID)
            file->header.nextFormID = lowestID;
      }
      for (auto* stub : this->forms_needing_flag) {
         this->_prep_for_delete(*stub, true);
         //
         stub->load()->friendly_delete_override(this->owner);
         stub->set_edited(true);
      }
      this->done = true;
   }
}