#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

namespace dovah {
   class form_stub;
}

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class game_setting_overrides_a_real_form final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr game_setting_overrides_a_real_form(const std::string_view sn, form_stub& of) : setting_name(sn), overridden_form(of) {}

         std::string setting_name;
         form_stub&  overridden_form;
         std::string overridden_file;
   };
}
#include "../_util.undef.h"