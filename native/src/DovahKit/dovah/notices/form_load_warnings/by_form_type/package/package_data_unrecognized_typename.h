#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::package {
   class paackage_data_unrecognized_typename final : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr paackage_data_unrecognized_typename(
            form_stub& subject,
            size_t which,
            std::string_view type
         )
         :
            base_form_load_warning(subject),
            which(which),
            type(type)
         {}

         size_t which; // the N-th package data
         std::string type;
   };
}
#include "../../../_util.undef.h"