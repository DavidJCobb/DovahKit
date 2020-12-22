#pragma once
#include "form.h"

namespace editor_script::wrappers {
   struct voicetype : public form {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.voicetype";
      static luaL_Reg metatable_methods[];
   };
}