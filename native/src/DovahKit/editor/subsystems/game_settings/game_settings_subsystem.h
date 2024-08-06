#pragma once
#include <cstdint>
#include <string_view>
#include <variant>
#include <QObject>
#include "helpers/singleton_ex.h"
#include "dovah/localized_strings.h"

namespace dovahkit::subsystems::game_settings {
   using game_setting_value = std::variant<
      std::monostate,
      bool,
      float,
      int32_t,
      dovah::localized_string
   >;

   class core;
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
         ~core();

      public:
         game_setting_value get_setting_value(const char* name);
         void set_setting_value(const char* name, const game_setting_value&); // may throw dovah::exceptions::game_setting_value_change_failed

      signals:
         void settingValueChanged(const char* name);
   };
}
