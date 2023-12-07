#pragma once
#include "./attached_script.h"
#include "dovah/data/papyrus/helpers/name_equals.h"

namespace dovah::loaded_forms::components::papyrus {
   constexpr bool attached_script::name_matches(const std::string_view& v) const {
      return dovah::papyrus::helpers::name_equals(v, std::string_view(this->name.data(), this->name.size()));
   }
   constexpr bool attached_script::name_matches(const std::string& v) const {
      return this->name_matches(std::string_view(v.data(), v.size()));
   }

   constexpr const property* attached_script::lookup_property(const std::string_view& name) const {
      for (auto& item : this->properties)
         if (dovah::papyrus::helpers::name_equals(name, item.name))
            return &item;
      return nullptr;
   }
   constexpr const property* attached_script::lookup_property(const std::string& name) const {
      return lookup_property(std::string_view(name.data(), name.size()));
   }
   constexpr property* attached_script::lookup_property(const std::string_view& name) {
      return const_cast<property*>(std::as_const(*this).lookup_property(name));
   }
   constexpr property* attached_script::lookup_property(const std::string& name) {
      return const_cast<property*>(std::as_const(*this).lookup_property(name));
   }
}