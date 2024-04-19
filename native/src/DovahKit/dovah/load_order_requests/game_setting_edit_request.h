#pragma once
#include <string>
#include "../data/game_settings.h" // game_setting_value
#include "../core.h"
#include "../notice_code_t.h"

namespace dovah {
   class file_load_order;
}

namespace dovah {
   class game_setting_edit_request {
      friend class file_load_order;
      public:
         enum class form_id_policy {
            find_valid_id,
            use_chosen_id,
         };

      protected:
         file_load_order& owner;
         bare_form_id_t   desiredID = 0;
         notice_code_t    code      = default_notice_code;
         bool reservedID = false;
         bool done       = false;
         
         game_setting_edit_request(file_load_order& o, form_id_policy);
         game_setting_edit_request(game_setting_edit_request&&);
         game_setting_edit_request(const game_setting_edit_request&) = delete;
         game_setting_edit_request& operator=(const game_setting_edit_request&) = delete;
         
      public:
         ~game_setting_edit_request();
         
         struct {
            std::string        name;
            game_setting_value value;
         } setting;
         const form_id_policy policy = form_id_policy::find_valid_id;
         
         constexpr bare_form_id_t get_queued_form_id() const noexcept { return this->desiredID; }
         constexpr notice_code_t get_notice_code() const noexcept { return this->code; }
         constexpr bool was_successful() const noexcept { return this->done; }
         
         void acquire_form_id(); // for use with form_id_policy::find_valid_id
         void set_desired_form_id(bare_form_id_t); // for use with form_id_policy::use_chosen_id
         
         void commit();
   };
}