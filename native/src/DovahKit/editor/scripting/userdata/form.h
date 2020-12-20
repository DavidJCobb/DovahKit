#pragma once
#include "_base.h"
#include "../../../dovah/core.h"

namespace editor_script::classes {
   class form : public _base {
      protected:
         virtual bool _is_equal_impl(const _base* other) const noexcept override { return true; }
      public:
         static constexpr char* superclass_key = metatable_key;
         static constexpr char* metatable_key  = "dovah.classes.form";
         static luaL_Reg metatable_methods[];
         //
         form(dovah::form_stub* s) {
            this->type = userdata_base_type::form_data;
            this->stub = s;
         }
   };
}