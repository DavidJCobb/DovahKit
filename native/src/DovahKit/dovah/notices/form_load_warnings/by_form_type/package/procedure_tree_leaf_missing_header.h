#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class procedure_tree_leaf_missing_header final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr procedure_tree_leaf_missing_header(
            form_stub& subject,
            uint32_t signature
         )
         :
            base_form_load_warning(subject),
            signature(signature)
         {}

         uint32_t signature;
   };
}
#include "../../../_util.undef.h"