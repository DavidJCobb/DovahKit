#pragma once
#include <initializer_list>
#include "../../../Lua/lua.hpp"

namespace editor_script {
   //
   // If the element at (stack_pos) is an instance of the class described by the 
   // (class_internal_name), then this returns the userdata pointer; otherwise, it 
   // returns nullptr. This only works for userdata, not for tables.
   //
   extern void* cast_to_class(lua_State* luaVM, int stack_pos, const char* class_internal_name);

   //
   // Same as (cast_to_class), except that it does not allow subclasses of the 
   // desired class; the userdata on the stack must be an exact match. Suitable 
   // for classes that you don't intend to ever subclass. This only works for 
   // userdata, not for tables.
   //
   extern void* cast_to_exact_class(lua_State* luaVM, int stack_pos, const char* class_internal_name);

   //
   // Similar to (cast_to_class), but returns a bool, and can be used on both 
   // userdata and normal tables. This is useful for things like utility classes 
   // backed by native code: scripts can be given the ability to instantiate 
   // normal tables using the class metatables, and this function can then be 
   // used by native code to validate these tables.
   //
   extern bool check_for_class(lua_State*, int stack_pos, const char* class_internal_name);

   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(
      lua_State* luaVM,
      const char* class_internal_name,
      const char* superclass_internal_name = nullptr,
      const std::initializer_list<luaL_Reg>& methods = {},
      const std::initializer_list<luaL_Reg>& getters = {},
      const std::initializer_list<luaL_Reg>& setters = {}
   );

   //
   // Check registry key (class_internal_name) for a class metatable.
   //
   extern bool is_class_defined(lua_State* luaVM, const char* class_internal_name);
}