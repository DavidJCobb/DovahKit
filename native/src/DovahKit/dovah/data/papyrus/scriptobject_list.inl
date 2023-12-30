#pragma once
#include "./scriptobject_list.h"
#include "./helpers/name_equals.h"

namespace dovah::papyrus {
   constexpr const scriptobject* scriptobject_list::get_script_by_name(const std::string_view& scriptname) const {
      for (const auto& item : this->objects)
         if (helpers::name_equals(scriptname, item.scriptname))
            return &item;
      return nullptr;
   }
   constexpr scriptobject* scriptobject_list::get_script_by_name(const std::string_view& scriptname) {
      return const_cast<scriptobject*>(std::as_const(*this).get_script_by_name(scriptname));
   }

   constexpr scriptobject* scriptobject_list::add_script(const std::string_view& scriptname) {
      if (auto* prior = this->get_script_by_name(scriptname)) {
         if (prior->inheritance.removed_on_target) {
            prior->inheritance.present_on_target = true;
            prior->inheritance.removed_on_target = false;
            return prior;
         }
         return nullptr;
      }
      auto& item = this->objects.emplace_back();
      item.scriptname = scriptname;
      item.inheritance.present_on_target = true;
      return &item;
   }

   constexpr void scriptobject_list::remove_script(const std::string_view& scriptname) {
      size_t i;
      {
         bool found = false;
         for (i = 0; i < this->objects.size(); ++i) {
            auto& item = this->objects[i];
            if (helpers::name_equals(scriptname, item.scriptname)) {
               found = true;
               break;
            }
         }
         if (!found)
            return;
      }
      auto& item = this->objects[i];
      if (item.inheritance.present_on_base) {
         item.inheritance.present_on_target = true;
         item.inheritance.removed_on_target = true;
         //
         // TODO: Should we clear overridden properties?
         //
      } else {
         this->objects.erase(this->objects.begin() + i);
      }
   }

   constexpr size_t scriptobject_list::get_script_count() const {
      return this->objects.size();
   }
   
   constexpr const scriptobject* scriptobject_list::get_nth_script(size_t n) const {
      if (n >= this->objects.size())
         return nullptr;
      return &this->objects[n];
   }
   constexpr scriptobject* scriptobject_list::get_nth_script(size_t n) {
      return const_cast<scriptobject*>(std::as_const(*this).get_nth_script(n));
   }
}