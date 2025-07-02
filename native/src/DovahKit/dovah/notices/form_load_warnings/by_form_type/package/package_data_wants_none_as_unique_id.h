#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"
#include "dovah/data/packages/package_data_type.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class package_data_wants_none_as_unique_id final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_data_wants_none_as_unique_id(
            form_stub& subject,
            size_t which,
            packages::package_data_type type
         )
         :
            base_form_load_warning(subject),
            which(which),
            data_type(type)
         {}

         packages::package_data_type data_type = packages::package_data_type::invalid;
         size_t which; // the N-th package data
   };
}
#include "../../../_util.undef.h"