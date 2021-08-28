#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Voicetype;
}

namespace dovahscript::wrappers {
   struct voicetype : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.voicetype";
      static constexpr const char*   class_name      = "voicetype";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Voicetype;
   };
}