#pragma once
#include <initializer_list>
#include <vector>
#include "../../lua.h"

namespace dovahscript::classes {
   //
   // If the element at (stack_pos) is an instance of the class described by the 
   // (class_internal_name), then this returns the userdata pointer; otherwise, it 
   // returns nullptr. This only works for userdata, not for tables.
   //
   [[nodiscard]] extern void* cast_to_class(lua_State* L, int stack_pos, const char* class_metatable_key);

   //
   // Same as (cast_to_class), except that it does not allow subclasses of the 
   // desired class; the userdata on the stack must be an exact match. Suitable 
   // for classes that you don't intend to ever subclass. This only works for 
   // userdata, not for tables.
   //
   [[nodiscard]] extern void* cast_to_exact_class(lua_State* L, int stack_pos, const char* class_metatable_key);

   //
   // Similar to (cast_to_class), but returns a bool, and can be used on both 
   // userdata and normal tables. This is useful for things like utility classes 
   // backed by native code: scripts can be given the ability to instantiate 
   // normal tables using the class metatables, and this function can then be 
   // used by native code to validate these tables.
   //
   [[nodiscard]] extern bool check_for_class(lua_State*, int stack_pos, const char* class_metatable_key);
   
   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(
      lua_State* L,
      const char* class_metatable_key,
      const std::initializer_list<const char*>& superclass_metatable_keys,
      const std::initializer_list<luaL_Reg>& methods = {},
      const std::initializer_list<luaL_Reg>& getters = {},
      const std::initializer_list<luaL_Reg>& setters = {},
      const char* class_name = nullptr // __name value for class metatable; defaults to (class_metatable_key) if not specified
   );

   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(
      lua_State* L,
      const char* class_metatable_key,
      const char* superclass_metatable_key = nullptr,
      const std::initializer_list<luaL_Reg>& methods = {},
      const std::initializer_list<luaL_Reg>& getters = {},
      const std::initializer_list<luaL_Reg>& setters = {},
      const char* class_name = nullptr // __name value for class metatable; defaults to (class_metatable_key) if not specified
   );

   extern void extend_class(
      lua_State* L,
      const char* class_metatable_key,
      const std::vector<luaL_Reg>& methods,
      const std::vector<luaL_Reg>& getters,
      const std::vector<luaL_Reg>& setters
   );

   // Pushes the following tables to the Lua stack: the class metatable; the class's 
   // getter list; and the class's setter list. If either of the latter two lists is 
   // missing (which can happen if the class had no [gs]etters), they will be created. 
   // Returns the number of values pushed to the Lua stack (0 in case of failure).
   [[nodiscard]] extern int get_class_tables(
      lua_State* L,
      const char* class_metatable_key
   );

   //
   // Check registry key (class_internal_name) for a class metatable.
   //
   [[nodiscard]] extern bool is_class_defined(lua_State* L, const char* class_metatable_key);
}