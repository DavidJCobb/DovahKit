#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class unexpected_subrecord_in_unique_id_list final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr unexpected_subrecord_in_unique_id_list(
            form_stub& subject,
            size_t which,
            uint32_t signature
         )
         :
            base_form_load_warning(subject),
            which(which),
            signature(signature)
         {}

         size_t   which; // the N-th subrecord that we expected to be a UNAM
         uint32_t signature;
   };
}
#include "../../../_util.undef.h"