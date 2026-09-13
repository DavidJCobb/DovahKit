#include "./fail_if_form_cannot_be_edited.h"
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/can_ever_be_edited.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "helpers/lua/error.h"
#include "../../lua.h"

namespace dovahscript::api_helpers {
   extern void fail_if_form_cannot_be_edited(lua_State* L, const dovah::form_stub* stub) {
      if (!stub)
         return;
      if (!dovah::form_stub_helpers::can_ever_be_edited(*stub)) {
         auto str = editor_helpers::form_identifiers_to_string(stub);
         cobb::lua::error(L, "Form %1 cannot be edited.", str.toStdString());
      }
   }
}