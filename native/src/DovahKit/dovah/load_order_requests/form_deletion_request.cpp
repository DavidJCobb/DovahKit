#include "./form_deletion_request.h"
#include "../files/tes_file_reading/file_loader.h"
#include "../files/file_load_order.h"
#include "../forms/Form.h"
#include "../form_stub.h"
#include "../exceptions/form_deletion_failed.h"

namespace {
   using exception  = dovah::exceptions::form_deletion_failed;
   using error_code = exception::error_code;
}

namespace dovah {
   form_deletion_request::form_deletion_request(file_load_order& o, form_stub& t) : owner(o), target(t) {
      if (!owner.active_file)
         throw exception(error_code::no_active_file, this->target);
      if (this->target.is_hardcoded() || this->target.formID < minimum_plugin_form_id)
         throw exception(error_code::form_is_hardcoded, this->target);

      this->active_file_prefix = this->owner.active_file_prefix();
      
      if (_form_should_be_flagged(this->target)) {
         this->forms_needing_flag.insert(&this->target);
      } else {
         this->forms_needing_delete.insert(&this->target);
      }
      this->seen_stubs.insert(&this->target);
      this->_gather_others(&this->target);
   }
   form_deletion_request::form_deletion_request(form_deletion_request&& other) : owner(other.owner), target(other.target) {
      std::swap(this->forms_needing_delete, other.forms_needing_delete);
      std::swap(this->seen_stubs, other.seen_stubs);
      //
      this->force_delete_overrides = other.force_delete_overrides;
      //
      this->active_file_prefix = this->owner.active_file_prefix();
   }
   bool form_deletion_request::_form_should_be_flagged(form_stub& stub) noexcept {
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
      if (!start)
         start = &this->target;
      //
      if (!start->load()) {
         auto ex = exception(error_code::unimplemented_form_type, this->target);
         ex.details.referent = start;
         throw ex;
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
            auto ex = exception(error_code::cannot_load_all_users_of_this_form, this->target);
            ex.details.referent = start;
            ex.details.referrer = entry.other;
            throw ex;
         }
         //
         bool should_delete = false;
         if (entry.flags & use_info_entry::flag::parent_child) {
            assert(!entry.other->is_hardcoded() && "How is a hardcoded form a descendant of a form that can be deleted (and in fact is currently being deleted)?");
            should_delete = true;
         } else if (this->delete_dialogue_children) {
            //
            // Quests aren't literally the parents of their contained dialogue branches 
            // and topics, but the design for dialogue is that all dialogue is parented 
            // to a quest, and all dialogue-tree topics are parented to a branch. By 
            // default, deleting either of these "pseudo-parents" will also trigger the 
            // deletion of their "pseudo-children" and downward.
            //
            switch (start->form_type) {
               case dovah::form_type::quest:
                  if (entry.flags & use_info_entry::flag::dialogue_quest) {
                     switch (entry.other->form_type) {
                        case dovah::form_type::dialogue_branch:
                        case dovah::form_type::topic:
                           should_delete = true;
                           break;
                     }
                  }
                  break;
               case dovah::form_type::dialogue_branch:
                  if (entry.flags & use_info_entry::flag::dialogue_branch) {
                     if (entry.other->form_type == dovah::form_type::topic)
                        should_delete = true;
                  }
                  break;
            }
         }
         //
         if (should_delete) {
            if (_form_should_be_flagged(*entry.other)) {
               this->forms_needing_flag.insert(entry.other);
            } else {
               this->forms_needing_delete.insert(entry.other);
            }
            this->_gather_others(entry.other);
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

   //
   // This operation is not allowed to fail, as there's no way to recover from any potential 
   // failures: we can't easily signal which deletion failed, and some other deletions may 
   // have already been carried out.
   //
   void form_deletion_request::commit() noexcept {
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
   }
}