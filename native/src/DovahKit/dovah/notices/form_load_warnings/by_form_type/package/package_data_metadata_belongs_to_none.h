#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class package_data_metadata_belongs_to_none final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_data_metadata_belongs_to_none(
            form_stub& subject,
            size_t which
         )
         :
            base_form_load_warning(subject),
            which(which)
         {}

         size_t which; // the N-th UNAM in the BGSPackageDataNameMap
   };
}
#include "../../../_util.undef.h"