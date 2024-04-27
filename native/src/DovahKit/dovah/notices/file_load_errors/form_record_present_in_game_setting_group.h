#pragma once
#include "./base_record_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class form_record_present_in_game_setting_group : public base_record_load_error {
      public:
         MAKE_ERROR_OVERLOADS;
   };
}
#include "../_util.undef.h"