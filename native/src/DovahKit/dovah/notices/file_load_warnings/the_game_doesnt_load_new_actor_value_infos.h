#pragma once
#include <cstdint>
#include <optional>
#include "../base_file_load_warning.h"
#include "../../core.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   class the_game_doesnt_load_new_actor_value_infos final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr the_game_doesnt_load_new_actor_value_infos(form_stub& subject) : subject(subject) {}

         // NOTE: At the time this warning is emitted, the `form_stub` instance may not have 
         //       a normalized form ID. Use the `bare_form_id_t` member.

         form_stub&     subject;
         bare_form_id_t subject_form_id = 0;
   };
}
#include "../_util.undef.h"