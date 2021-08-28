#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class TextureSet;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc texture_set_paths = "TxStPath";
}

namespace dovahscript::wrappers {
   struct texture_set : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.texture_set";
      static constexpr const char*   class_name      = "texture_set";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::TextureSet;
   };
}