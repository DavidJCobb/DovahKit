#include "unknown.h"

namespace {
   using namespace editor_script;
   using cls = wrappers::resource::unknown;
}

namespace editor_script::wrappers::resource {
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_methods = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> cls::metatable_setters = no_functions;
}