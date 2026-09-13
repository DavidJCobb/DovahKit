#include "./form_deletion_request.h"
#include "../files/tes_file_reading/file_loader.h"
#include "../files/file_load_order.h"
#include "../forms/Form.h"
#include "../form_stub.h"
#include "../form_stub_addenda.h"
#include "../form_stub_addenda/passkeys/ordered_child_collection.h"
#include "../exceptions/form_deletion_failed.h"
#include "../load_order_processes/file_save.h"
#include "../forms/factories/construct.h" // can_load_form_data
#include "../use_info/entry_flag_to_mask.h"
#include "../use_info/entry_flags/base.h"
#include "../use_info/entry_flags/dialogue_branch.h"
#include "../use_info/entry_flags/scene.h"
#include "../use_info/entry_flags/topic.h"

namespace {
   using exception  = dovah::exceptions::form_deletion_failed;
   using error_code = exception::error_code;
}

// We could define a template function on `form_deletion_request` which returns a `loaded_form_ptr` 
// based on this expression, but then we'd either have to include `form_stub.h` or forward-declare 
// the loaded-form-pointer template, and I just... don't much feel like doing either right now. The 
// former makes the form stub header more viral, and the latter is brittle (because I want to take 
// the loaded-form-pointer template and refactor it in the future (because it sucks right now)).
#define LOAD_FORM_FROM_STUB(stub) (this->is_mid_save_cleanup ? (stub)->load_even_if_unsafe({}) : (stub)->load())

namespace dovah {
   form_deletion_request::form_deletion_request(file_save_passkey, load_order_processes::file_save& process, form_stub& t)
      : owner(process.active_load_order), target(t)
   {
      this->is_mid_save_cleanup = true;
      this->_common_init();
   }
   form_deletion_request::form_deletion_request(file_load_order& o, form_stub& t) : owner(o), target(t) {
      this->_common_init();
   }
   form_deletion_request::form_deletion_request(form_deletion_request&& other) noexcept : owner(other.owner), target(other.target) {
      std::swap(this->forms_needing_delete, other.forms_needing_delete);
      std::swap(this->seen_stubs, other.seen_stubs);
      //
      this->is_mid_save_cleanup = other.is_mid_save_cleanup;
      //
      this->active_file_prefix = this->owner.expected_active_file_prefix_post_save();
   }
   void form_deletion_request::_common_init() {
      if (!this->owner.active_file)
         throw exception(error_code::no_active_file, this->target);
      if (this->target.is_hardcoded())
         throw exception(error_code::form_is_hardcoded, this->target);

      this->active_file_prefix = this->owner.expected_active_file_prefix_post_save();
      
      if (_form_should_be_flagged(this->target)) {
         this->forms_needing_flag.insert(&this->target);
      } else {
         this->forms_needing_delete.insert(&this->target);
      }
      this->seen_stubs.insert(&this->target);
      this->_gather_others(&this->target);
   }
   bool form_deletion_request::_form_should_be_flagged(form_stub& stub) noexcept {
      if (stub.is_hardcoded()) // we don't currently allow any kind of deletion of hardcoded forms, but it never hurts to be prepared for what might change
         return true;
      if (!this->active_file_prefix.contains_form_id(stub.formID)) {
         if (stub.get_file_at_index(0) == this->owner.active_file)
            //
            // A form can be defined in the active file yet stored outside of the active 
            // file's ID range if it's an injected record.
            //
            return false;
         return true;
      }
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
      // Don't allow deletion of forms if we don't know how to load the form types in question. Do 
      // allow deletion of none-stubs, since they aren't "real" forms: when we save the active file, 
      // any none-stubs that have since become unreferenced are deleted per this process here.
      // 
      // (None-stubs are created as a means to track dangling uses and prevent newly-created forms 
      // from "filling in" the used form IDs. During save, dangling uses may be cleared, and that 
      // leads to us invoking the normal form-deletion process on the none-stub. If a frontend runs 
      // a deletion request on a none-stub outside of the save process, that should just result in 
      // us forcibly severing uses of the none-stub just as we would any other to-be-deleted form.)
      //
      if (!start->is_none_stub() && !can_load_form_data(start->form_type)) {
         auto ex = exception(error_code::unimplemented_form_type, this->target);
         ex.details.referent = start;
         throw ex;
      }

      for (auto& pair : start->inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         if (this->seen_stubs.find(other) != this->seen_stubs.end())
            continue;
         this->seen_stubs.insert(entry.other);
         //
         if (!LOAD_FORM_FROM_STUB(entry.other)) { // Check to ensure we can load all users.
            auto ex = exception(error_code::cannot_load_all_users_of_this_form, this->target);
            ex.details.referent = start;
            ex.details.referrer = entry.other;
            throw ex;
         }
         //
         bool should_delete = false;
         if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)) {
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
                  switch (entry.other->form_type) {
                     case dovah::form_type::dialogue_branch:
                        should_delete = (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::dialogue_branch::parent_quest));
                        break;
                     case dovah::form_type::scene:
                        should_delete = (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::scene::parent_quest));
                        break;
                     case dovah::form_type::topic:
                        should_delete = (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::topic::parent_quest));
                        break;
                  }
                  break;
               case dovah::form_type::dialogue_branch:
                  if (entry.other->form_type == dovah::form_type::topic)
                     should_delete = (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::topic::parent_branch));
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
      if (auto* parent = stub.get_parent_form()) { // doing this here is a HACK
         if (auto* addenda = parent->addenda) {
            addenda->ordered_children._sever_references_to_deleted_form({}, stub, true);
         }
      }
      stub.sever_all_outbound_references();
      
      std::vector<std::pair<form_stub*, bool>> pending;
      //
      // Gather the forms first. Processing them as we gather them would change use info 
      // and potentially invalidate iterators.
      //
      for (auto& pair : stub.inbound) {
         auto& entry = pair.second;
         auto* other = entry.other;
         pending.push_back({ other, (bool)(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)) });
      }
      for (auto& [user, user_is_child] : pending) {
         user->set_edited(true);
         {
            auto form = LOAD_FORM_FROM_STUB(user);
            assert(!!form); // our caller, form_deletion_request::commit, is not allowed to fail
            form->sever_outbound_references_to(stub);
         }
         if (user_is_child)
            user->orphan();
         if (user->addenda)
            user->addenda->sever_references_to_deleted_form(stub, flag);
      }
   }

   void form_deletion_request::set_is_mid_save_cleanup(file_save_passkey) {
      this->is_mid_save_cleanup = true;
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
         LOAD_FORM_FROM_STUB(stub)->friendly_delete_override(this->owner);
         stub->set_edited(true);
      }
   }
}