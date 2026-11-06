#pragma once
#include <stdexcept>
#include <string>

namespace dovah::exceptions {
   class game_setting_edit_failed : public std::runtime_error {
      public:
         enum class error_code {
            no_active_file,

            no_form_id_available,
         };

      public:
         game_setting_edit_failed(
            error_code code,
            const std::string& setting_name
         ) :
            std::runtime_error("game setting edit failed"),
            code(code),
            setting_name(setting_name)
         {}

         const error_code code;
         std::string      setting_name;
   };
}