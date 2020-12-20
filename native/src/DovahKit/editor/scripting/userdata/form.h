#pragma once
#include "_base.h"
#include "../../../dovah/core.h"

namespace editor_script::classes {
   class form : public _base {
      public:
         static constexpr char* metatable_key = "dovah.classes.form";
         static luaL_Reg metatable_methods[];
         //
         dovah::form_stub* stub = nullptr;
   };
}