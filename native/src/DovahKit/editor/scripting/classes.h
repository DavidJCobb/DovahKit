#pragma once
#include "../../../Lua/lua.hpp"

namespace editor_script {
   //
   // If the element at (stack_pos) is an instance of the class described by the 
   // (class_internal_name), then this returns the userdata pointer; otherwise, it 
   // returns nullptr.
   //
   extern void* cast_to_class(lua_State* luaVM, int stack_pos, const char* class_internal_name);

   //
   // Same as (cast_to_class), except that it does not allow subclasses of the 
   // desired class; the userdata on the stack must be an exact match. Suitable 
   // for classes that you don't intend to ever subclass.
   //
   extern void* cast_to_exact_class(lua_State* luaVM, int stack_pos, const char* class_internal_name);

   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(lua_State* luaVM, const char* class_internal_name, const char* superclass_internal_name = nullptr, const luaL_Reg* methods = nullptr, const luaL_Reg* getters = nullptr, const luaL_Reg* setters = nullptr);

   //
   // Check registry key (class_internal_name) for a class metatable.
   //
   extern bool is_class_defined(lua_State* luaVM, const char* class_internal_name);
}