#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/package_data_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class package_data_unexpected_value_subrecord final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_data_unexpected_value_subrecord(
            form_stub& subject,
            size_t which,
            packages::package_data_type type,
            uint32_t signature
         )
         :
            base_form_load_warning(subject),
            which(which),
            data_type(type),
            signature(signature)
         {}

         packages::package_data_type data_type = packages::package_data_type::invalid;
         size_t which; // the N-th package data
         uint32_t signature;
   };
}
#include "../../../_util.undef.h"