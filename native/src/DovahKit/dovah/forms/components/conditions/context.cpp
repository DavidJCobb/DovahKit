#include "./context.h"

#include "../../../form_stub.h"
#include "../../Package.h"
#include "../../Quest.h"

namespace dovah::loaded_forms::components::conditions {
   context::context(form_stub& owner, bool prefer_working_copy) : owner(&owner), prefer_working_copy(prefer_working_copy) {
      if (owner.form_type == form_type::quest) {
         this->quest = &owner;
      } else if (owner.form_type == form_type::package) {
         this->package = &owner;
         //
         // TODO: get owning quest
         //
      } else if (owner.form_type == form_type::scene) {
         //
         // TODO: get owning quest
         //
      } else if (owner.form_type == form_type::topic) {
         auto* quest = owner.get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
         if (quest->form_type == dovah::form_type::quest)
            this->quest = quest;
      } else if (owner.form_type == form_type::topic_info) {
         auto* parent = owner.get_parent_form();
         if (parent && parent->form_type == dovah::form_type::topic) {
            auto* quest = parent->get_outbound_use_with_flag(dovah::use_info_entry::flag::dialogue_quest);
            if (quest->form_type == dovah::form_type::quest)
               this->quest = quest;
         }
      }
      //
      if (auto* s = this->package) {
         if (s->form_type != form_type::package)
            this->package = nullptr;
         else
            this->loaded.package = s->load().ptr_cast<loaded_forms::Package>();
      }
      if (auto* s = this->quest) {
         if (s->form_type != form_type::quest)
            this->quest = nullptr;
         else
            this->loaded.quest = s->load().ptr_cast<loaded_forms::Quest>();
      }
   }

   loaded_forms::Package* context::get_owning_package() const noexcept {
      if (!this->package)
         return nullptr;
      if (this->prefer_working_copy) {
         auto* wc = this->package->get_working_copy<loaded_forms::Package>();
         if (wc)
            return wc;
      }
      return this->loaded.package.unwrap();
   }
   loaded_forms::Quest* context::get_owning_quest() const noexcept {
      if (!this->quest)
         return nullptr;
      if (this->prefer_working_copy) {
         auto* wc = this->quest->get_working_copy<loaded_forms::Quest>();
         if (wc)
            return wc;
      }
      return this->loaded.quest.unwrap();
   }
}