#include "./context.h"

#include "../../../form_stubs/helpers/get_unique_outbound_use.h"
#include "../../../form_stub.h"
#include "../../../use_info/entry_flags/topic.h"
#include "../../Package.h"
#include "../../Quest.h"
#include "../../Scene.h"

namespace dovah::loaded_forms::components::conditions {
   context::context(form_stub& owner, bool prefer_working_copy) : owner(&owner), prefer_working_copy(prefer_working_copy) {
      if (owner.form_type == form_type::quest) {
         this->quest = &owner;
      } else if (owner.form_type == form_type::package) {
         this->package = &owner;
         //
         // We'll defer getting the owning quest 'til later, since we have to 
         // keep the package loaded anyway.
         //
      } else if (owner.form_type == form_type::scene) {
         auto loaded = owner.load().ptr_cast<loaded_forms::Scene>();
         if (loaded) {
            this->quest = loaded->owning_quest.get_form_stub();
         }
      } else if (owner.form_type == form_type::topic) {
         auto* quest = form_stub_helpers::get_unique_outbound_use<use_info::entry_flags::topic::parent_quest>(owner);
         if (quest->form_type == dovah::form_type::quest)
            this->quest = quest;
      } else if (owner.form_type == form_type::topic_info) {
         auto* parent = owner.get_parent_form();
         if (parent && parent->form_type == dovah::form_type::topic) {
            auto* quest = form_stub_helpers::get_unique_outbound_use<use_info::entry_flags::topic::parent_quest>(*parent);
            if (quest->form_type == dovah::form_type::quest)
               this->quest = quest;
         }
      }
      //
      if (auto* s = this->package) {
         this->update_from_owning_package();
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

   void context::update_from_owning_package() {
      if (!this->package)
         return;
      if (this->package->form_type != dovah::form_type::package) {
         this->package = nullptr;
         this->loaded.package = {};
         if (this->owner == this->package) {
            this->quest = nullptr;
            this->loaded.quest = {};
         }
         return;
      }
      if (!this->loaded.package || &this->loaded.package->stub != this->package) {
         this->loaded.package = this->package->load().ptr_cast<loaded_forms::Package>();
         if (!this->loaded.package) {
            if (this->owner == this->package) {
               this->quest = nullptr;
               this->loaded.quest = nullptr;
            }
            return;
         }
      }
      this->quest = this->loaded.package->owning_quest.get_form_stub();
      if (this->prefer_working_copy) {
         auto* wc = this->package->get_working_copy<loaded_forms::Package>();
         if (wc)
            this->quest = wc->owning_quest.get_form_stub();
      }
      if (this->quest) {
         if (this->quest->form_type != dovah::form_type::quest) {
            this->quest = nullptr;
            this->loaded.quest = nullptr;
            return;
         }
         this->loaded.quest = this->quest->load().ptr_cast<loaded_forms::Quest>();
      }
   }
}