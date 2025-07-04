#pragma once
#include "./_target_base.h"

namespace dovah::loaded_forms::structs::custom_packages {
   class package_data_target_selector : public package_data_target_base {
      public:
         static constexpr const char* const serialized_typename = "TargetSelector";

      public:
         virtual package_data_type get_type() const noexcept { return package_data_type::target_selector; }
   };
}