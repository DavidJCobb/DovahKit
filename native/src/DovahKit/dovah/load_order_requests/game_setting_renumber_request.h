#pragma once
#include <string>
#include "../core.h"

namespace dovah {
   class file_load_order;
}

namespace dovah {
   class game_setting_renumber_request {
      friend class file_load_order;
      protected:
         file_load_order& owner;
         bare_form_id_t   desiredID = 0;
         bool reservedID = false;
         bool done       = false;
         
         game_setting_renumber_request(file_load_order& o);
         game_setting_renumber_request(game_setting_renumber_request&&);
         game_setting_renumber_request(const game_setting_renumber_request&) = delete;
         game_setting_renumber_request& operator=(const game_setting_renumber_request&) = delete;
         
      public:
         ~game_setting_renumber_request();
         
         std::string setting;
         
         constexpr bare_form_id_t get_queued_form_id() const noexcept { return this->desiredID; }
         constexpr bool was_successful() const noexcept { return this->done; }
         
         void set_desired_form_id(bare_form_id_t); // for use with form_id_policy::use_chosen_id
         
         void commit();
   };
}