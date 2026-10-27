#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <vector>
#include "lu/integral_character_code.h"
#include "../base_form_load_warning.h"
namespace dovah {
   enum class form_type : uint8_t;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::form_load_warnings {
   class form_use_is_wrong_type final : public base_form_load_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         struct additional_info {
            std::optional<size_t> nth_subrecord;
         };

      public:
         constexpr form_use_is_wrong_type(
            form_stub&  user,
            form_stub&  used,
            std::vector<form_type>&& desired,
            lu::four_cc subrecord_signature,
            const additional_info& ai = {}
         ) :
            base_form_load_warning(user),
            used_form(used),
            desired_form_types(std::move(desired)),
            subrecord_signature(subrecord_signature),
            additional_info(ai)
         {}

         form_stub& used_form;
         std::vector<form_type> desired_form_types;
         lu::four_cc    subrecord_signature;
         additional_info additional_info;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"