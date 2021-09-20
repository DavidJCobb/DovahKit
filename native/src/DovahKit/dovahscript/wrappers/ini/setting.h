#pragma once
#include "../base.h"
#include "../../wrapper.h"
#include "../../../helpers/qt/ini.h"

namespace dovahscript::wrappers::ini {
   struct setting : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key    = "dovah.classes.ini_setting";
      static constexpr const char*   class_name       = "ini_setting";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      static constexpr const char* global_name = "ini_setting";
   };
}