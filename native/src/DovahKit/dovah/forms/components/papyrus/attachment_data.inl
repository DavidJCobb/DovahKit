#pragma once
#include "./attachment_data.h"
#include "dovah/data/papyrus/helpers/name_equals.h"

namespace dovah::loaded_forms::components::papyrus {
   constexpr const attached_script* attachment_data::lookup_script(const std::string_view& name) const {
      for (auto& item : this->scripts)
         if (dovah::papyrus::helpers::name_equals(name, item.name))
            return &item;
      return nullptr;
   }
   constexpr const attached_script* attachment_data::lookup_script(const std::string& name) const {
      return lookup_script(std::string_view(name.data(), name.size()));
   }
   constexpr attached_script* attachment_data::lookup_script(const std::string_view& name) {
      return const_cast<attached_script*>(std::as_const(*this).lookup_script(name));
   }
   constexpr attached_script* attachment_data::lookup_script(const std::string& name) {
      return const_cast<attached_script*>(std::as_const(*this).lookup_script(name));
   }
}
