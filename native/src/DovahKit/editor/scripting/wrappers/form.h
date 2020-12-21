#pragma once
#include "../wrapper.h"

namespace editor_script::wrappers {
   class form : public wrapper {
      protected:
         virtual bool _is_equal_impl(const wrapper* other) const noexcept override { return true; }
      public:
         static constexpr char* superclass_key = metatable_key;
         static constexpr char* metatable_key  = "dovah.classes.form";
         static luaL_Reg metatable_methods[];
         //
         form(dovah::form_stub* s) {
            this->type = wrapper_type::form_data;
            this->stub = s;
         }
   };
}