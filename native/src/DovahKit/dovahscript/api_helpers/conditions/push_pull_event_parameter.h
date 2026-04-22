#pragma once
#include <cstdint>
#include <expected>
#include <string_view>
#include "dovah/data/conditions/event_function.h"
struct lua_State;
namespace dovah {
   class form_stub;
}

namespace dovahscript::api_helpers::conditions {
   extern std::expected<dovah::conditions::event_function::type, std::string_view> pull_event_function(lua_State* L, int pos);
   extern std::expected<uint16_t, std::string_view> pull_event_member(lua_State* L, int pos);
   extern std::expected<dovah::form_stub*, std::string_view> pull_event_form(lua_State* L, int pos);

   extern void push_event_function(lua_State*, uint16_t);
   extern void push_event_member(lua_State*, uint16_t);
   extern void push_event_form(lua_State*, dovah::form_stub*);
}