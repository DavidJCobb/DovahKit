#pragma once
#include "./scriptobject.h"
#include "./helpers/name_equals.h"

namespace dovah::papyrus {
   constexpr const property* scriptobject::_get_property_by_name(const std::string_view& name) const {
      for (auto& item : this->_properties)
         if (dovah::papyrus::helpers::name_equals(name, item.name()))
            return &item;
      return nullptr;
   }
   constexpr property* scriptobject::_get_property_by_name(const std::string_view& name) {
      return const_cast<property*>(std::as_const(*this)._get_property_by_name(name));
   }

   property* scriptobject::_add_property(const std::string_view& name) {
      if (auto* prior = this->_get_property_by_name(name)) {
         if (prior->inheritance.removed_on_target) {
            prior->inheritance.present_on_target = true;
            prior->inheritance.removed_on_target = false;
            return prior;
         }
         return nullptr;
      }
      auto& item = this->_properties.emplace_back();
      item._name = name;
      item.inheritance.present_on_target = true;
      return &item;
   }
   void scriptobject::_remove_property(const std::string_view& name) {
      size_t i;
      {
         bool found = false;
         for (i = 0; i < this->_properties.size(); ++i) {
            auto& item = this->_properties[i];
            if (helpers::name_equals(name, item.name())) {
               found = true;
               break;
            }
         }
         if (!found)
            return;
      }
      auto& item = this->_properties[i];
      if (item.inheritance.present_on_base || item.inheritance.present_in_script) {
         item.inheritance.present_on_target = true;
         item.inheritance.removed_on_target = true;
      } else {
         this->_properties.erase(this->_properties.begin() + i);
      }
   }
}