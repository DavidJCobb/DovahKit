#pragma once
#include "./known_script.h"

namespace dovahkit::subsystems::papyrus {
   constexpr const known_script* known_script::superclass() const {
      if (this->info.loose.has_value())
         return this->info.loose.value().extends.target;
      if (this->info.packed.has_value())
         return this->info.packed.value().extends.target;
      return nullptr;
   }
   constexpr known_script* known_script::superclass() {
      return const_cast<known_script*>(std::as_const(*this).superclass());
   }
   constexpr std::optional<dovah::form_type_t> known_script::underlying_type() const {
      if (this->inheritance.root_class)
         return this->inheritance.root_class->underlying_type();

      if (this->info.loose.has_value())
         return this->info.loose.value().extends.underlying_type;
      if (this->info.packed.has_value())
         return this->info.packed.value().extends.underlying_type;

      return {};
   }
}