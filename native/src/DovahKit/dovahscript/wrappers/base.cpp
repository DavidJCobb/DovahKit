#include "base.h"
#include "../wrapper.h"

namespace dovahscript {
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_methods = {
      { "__close", &wrapper::__close },
      { "__gc",    &wrapper::__gc },
   };
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_getters = no_functions;
   /*static*/ const std::initializer_list<luaL_Reg> wrapper_metatable::metatable_setters = no_functions;
}