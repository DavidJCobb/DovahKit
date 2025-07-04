#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class package_data_header_missing final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr package_data_header_missing(
            form_stub& subject,
            size_t which,
            uint32_t signature
         )
         :
            base_form_load_warning(subject),
            which(which),
            signature(signature)
         {}

         size_t   which; // the N-th subrecord that we expected to be an ANAM
         uint32_t signature;
   };
}
#include "../../../_util.undef.h"