#include "./known_script.h"
#include "dovah/data/papyrus/helpers/name_equals.h"

namespace dovahkit::subsystems::papyrus {
   void known_script::receive_archived_subclass(subsystem_passkey, known_script& subclass) {
      auto& list = this->inheritance.potential_subclasses.packed;
      auto  it   = std::find(list.begin(), list.end(), &subclass);
      if (it != list.end())
         return;
      list.push_back(&subclass);
   }
   //
   void known_script::receive_loose_subclass(subsystem_passkey, known_script& subclass) {
      auto& list = this->inheritance.potential_subclasses.loose;
      auto  it   = std::find(list.begin(), list.end(), &subclass);
      if (it != list.end())
         return;
      list.push_back(&subclass);
   }
   void known_script::abandon_loose_subclass(subsystem_passkey, known_script& subclass) {
      auto& list = this->inheritance.potential_subclasses.loose;
      auto  it   = std::find(list.begin(), list.end(), &subclass);
      if (it == list.end())
         return;
      list.erase(it);
   }
   
   void known_script::_compute_root_class(subsystem_passkey) {
      this->inheritance.cyclical   = false;
      this->inheritance.root_class = nullptr;

      if (this->superclass() == this) {
         this->inheritance.cyclical = true;
         return;
      }

      std::vector<known_script*> seen;
      for (auto* script = this; script; script = script->superclass()) {
         //
         // Guard against cyclical inheritance.
         //
         if (script->inheritance.cyclical) {
            this->inheritance.cyclical = true;
            break;
         }
         {
            bool already_seen = false;
            {
               auto it = std::find(seen.begin(), seen.end(), script);
               already_seen = it != seen.end();
            }
            if (already_seen) {
               this->inheritance.cyclical = true;
               break;
            }
         }

         //
         // Else, we're fine.
         //
         if (auto* already_computed_root = script->inheritance.root_class) {
            seen.push_back(already_computed_root);
            break;
         }
         seen.push_back(script);
      }
      if (!this->inheritance.cyclical && seen.size() > 1) {
         this->inheritance.root_class = seen.back();
      }
   }
   void known_script::_update_descendants_root_class(subsystem_passkey) {
      for_each_descendant_class([this](known_script& child) {
         child.inheritance.root_class = this->inheritance.root_class;
      });
   }
}