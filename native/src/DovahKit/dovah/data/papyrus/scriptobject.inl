#pragma once
#include "./scriptobject.h"
#include "./helpers/name_equals.h"

namespace dovah::papyrus {
   constexpr const property* scriptobject::lookup_property(const std::string_view& name) const {
      for (auto& item : this->properties)
         if (dovah::papyrus::helpers::name_equals(name, item.name))
            return &item;
      return nullptr;
   }
   constexpr property* scriptobject::lookup_property(const std::string_view& name) {
      return const_cast<property*>(std::as_const(*this).lookup_property(name));
   }
}