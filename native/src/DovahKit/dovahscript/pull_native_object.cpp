#include "pull_native_object.h"
#include "../helpers/lua/error.h"
#include "../dovah/form_stub.h"
#include "wrapper.h"
#include "wrappers/form/form.h"

namespace dovahscript {
   [[nodiscard]] extern dovah::form_stub* pull_form_stub_argument(lua_State* L, int arg, dovah::form_type ft) {
      if (lua_isnoneornil(L, arg))
         return nullptr;
      auto* other = wrapper_from_stack<wrappers::form>(L, arg);
      cobb::lua::argcheck(L, other != nullptr, arg, "form or nil expected");
      if (ft != dovah::form_type::none)
         other->error_if_wrong_form_type(L, arg, ft, true);
      return other->stub;
   }
}