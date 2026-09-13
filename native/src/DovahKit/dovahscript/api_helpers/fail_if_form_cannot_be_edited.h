#pragma once
namespace dovah {
   class form_stub;
}
struct lua_State;

namespace dovahscript::api_helpers {
   extern void fail_if_form_cannot_be_edited(lua_State* L, const dovah::form_stub*);
}