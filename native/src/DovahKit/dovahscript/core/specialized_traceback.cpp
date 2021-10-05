#include "specialized_traceback.h"
#include <array>
#include <cassert>
#include <string>
#include "../../helpers/lua/find_key_of.h"
#include "../../helpers/lua/find_path_of.h"
#include "../../helpers/lua/raw.h"
#include "../../helpers/strings.h"

namespace {
   std::string _search_modules_for_function(lua_State* L, lua_Debug& record) {
      /*
      function _find_as_global(func)
         local GLOBAL_PREFIX <const> = "_G."
         --
         local modules = REGISTRY[LOADED_TABLE]
         local name    = _recursive_find_value_in_table(func, modules, 2)
         if name == "" then
            return name
         end
         if name:sub(1, #GLOBAL_PREFIX) == GLOBAL_PREFIX then -- if the path starts with "_G."
            --
            -- This can happen if the "base" module was loaded, as _G is 
            -- technically part of that module.
            --
            name = name:sub(#GLOBAL_PREFIX)
         end
         return name
      end
      */
      constexpr const char* global_prefix = LUA_GNAME ".";
      constexpr size_t      prefix_size   = cobb::strlen(global_prefix);
      //
      auto top  = lua_gettop(L);
      lua_getinfo (L, "f", &record); // push function
      lua_getfield(L, LUA_REGISTRYINDEX, LUA_LOADED_TABLE); // get all loaded modules
      auto name = cobb::lua::find_path_of(L, -1, top + 1, 2);
      if (name.compare(0, prefix_size, global_prefix) == 0)
         name.erase(0, prefix_size);
      lua_settop(L, top);
      return name;
   }

   // Functions in Lua don't actually have names. Lua error reporting will attempt 
   // to identify a function's name based on whether it originates from any loaded 
   // modules or (failing that) based on how the function was originally accessed.
   std::string _function_name(lua_State* L, lua_Debug& record) {
      auto name = _search_modules_for_function(L, record);
      if (!name.empty())
         return name;
      if (record.namewhat[0] != '\0') {
         name = record.namewhat;
         name += " '";
         name += record.name;
         name += '\'';
         return name;
      }
      if (record.what[0] == 'm') {
         return "main chunk";
      }
      if (record.what[0] != 'C') {
         name = "function <";
         name += record.short_src;
         name += ':';
         name += std::to_string(record.linedefined);
         name += '>';
         return name;
      }
      return "?";
   }

   bool _frame_is_c(lua_State* L, lua_Debug& record, int level) {
      if (!lua_getstack(L, level, &record))
         return false;
      lua_getinfo(L, "S", &record);
      return record.what[0] == 'C';
   }

   template<size_t N> inline bool _is_in_table_as(lua_State* L, int value_index, int table_index, std::array<const char*, N> names) {
      /*
      function _is_in_table_as(v, t, names)
         for i = 1, #names do
            local name = names[i]
            if t[name] == v then
               return true
            end
         end
         return false
      end
      */
      table_index = lua_absindex(L, table_index);
      value_index = lua_absindex(L, value_index);
      for (const auto name : names) {
         lua_pushstring(L, name);
         lua_rawget    (L, table_index);
         bool match = lua_rawequal(L, -1, value_index);
         lua_pop(L, 1);
         if (match)
            return true;
      }
      return false;
   }


   [[nodiscard]] extern std::string _identify_instance_member(lua_State* L, int instance_index, int metacall_index, int function_index) {
      constexpr auto fail = "";
      auto top = lua_gettop(L);
      //
      instance_index = lua_absindex(L, instance_index);
      metacall_index = lua_absindex(L, metacall_index); // __index, __newindex, etc.
      function_index = lua_absindex(L, function_index); // specific getter/setter/method that we wish to identify
      if (lua_type(L, instance_index) != LUA_TUSERDATA)
         return fail;
      if (lua_type(L, function_index) != LUA_TFUNCTION)
         return fail;
      if (!lua_getmetatable(L, instance_index))
         return fail;
      int metatable_index = lua_gettop(L);
      int classlist_index = metatable_index + 1;
      bool match = _is_in_table_as(L, metacall_index, metatable_index, std::array{ "__index", "__newindex" });
      if (!match) {
         lua_settop(L, top);
         return fail;
      }
      //
      std::string classname;
      cobb::lua::rawgetfield(L, metatable_index, "__name");
      classname = lua_tostring(L, -1);
      classname += '.';
      lua_pop(L, 1);
      //
      cobb::lua::rawgetfield(L, metatable_index, "__classlist");
      lua_Unsigned class_count = lua_rawlen(L, classlist_index);
      std::string  name;
      for (auto i = class_count; i > 0; --i) {
         lua_settop (L, classlist_index);
         lua_rawgeti(L, classlist_index, i); // push (super)class
         //
         name = cobb::lua::find_key_of(L, -1, function_index);
         if (!name.empty()) {
            name.insert(0, classname);
            name.insert(0, "method: ");
            break;
         }
         lua_pushstring(L, "__getters");
         lua_rawget    (L, -2);
         name = cobb::lua::find_key_of(L, -1, function_index);
         lua_pop(L, 1); // pop getter list
         if (!name.empty()) {
            name.insert(0, classname);
            name.insert(0, "getter: ");
            break;
         }
         lua_pushstring(L, "__setters");
         lua_rawget    (L, -2);
         name = cobb::lua::find_key_of(L, -1, function_index);
         lua_pop(L, 1); // pop setter list
         if (!name.empty()) {
            name.insert(0, classname);
            name.insert(0, "setter: ");
            break;
         }
      }
      lua_settop(L, top);
      return name;
   }
}

namespace dovahscript::core {
   extern void specialized_traceback(lua_State* L, const std::string& message, int level) {
      std::string output;
      auto top = lua_gettop(L);
      //
      if (!message.empty()) {
         output += message;
         output += '\n';
      }
      output += "stack traceback:";
      //
      lua_Debug record;
      {  // Identify class getters, setters, and member functions, and pretty-print them.
         //
         // If we throw an error or warning from inside of a class getter or setter, then 
         // the output of luaL_traceback will look like this:
         // 
         //    message text
         //    stack traceback:
         //       [C]: in ?
         //       [C]: in metamethod 'newindex'
         //       [string: "userscript"}:2: in main chunk
         // 
         // This is a pattern we can check for. Let's see if we can confirm the following 
         // conditions:
         // 
         //  - Topmost two stack frames are C-functions
         // 
         //  - First argument (self-object) to second stack frame is a userdata
         // 
         //  - Second stack frame is __index or __newindex on that self-object
         // 
         //  - First stack frame is a getter, setter, or method in that self-object's 
         //    metatable
         // 
         // If so, then maybe we can print something like this instead:
         // 
         //    message text
         //    stack traceback:
         //       [C]: in getter classname.member
         //       [string: "userscript"]:2: in main chunk
         //
         if (_frame_is_c(L, record, level)) {
            int top = lua_gettop(L);
            lua_getinfo(L, "f", &record); // get possible member
            if (_frame_is_c(L, record, level + 1)) {
               //
               // Next, we want to get the possible metamethod, as well as the "self" value. 
               // However, Lua calls are somewhat complicated: it seems that by the time we 
               // get here, the arguments and function originally on the stack for the call 
               // we're interested in will have been removed.
               // 
               // As such, I've modified my class implementations to copy the self-object 
               // onto the stack before setting up a call, such that when the call is made 
               // and produces stack changes, just the self-object is left on the stack, at 
               // position 1.
               //
               lua_getinfo(L, "f", &record); // get possible metamethod
               lua_getlocal(L, &record, 1);  // push self-object onto the stack
               auto descriptor = _identify_instance_member(L, -1, -2, -3);
               if (!descriptor.empty()) {
                  output += "\n\t[C]: in ";
                  output += descriptor;
                  level  += 2;
               }
            }
            lua_settop(L, top);
         }
      }
      while (lua_getstack(L, level++, &record)) {
         lua_getinfo(L, "nSlt", &record);
         output += "\n\t";
         output += record.short_src;
         output += ':';
         if (record.currentline > 0) {
            output += std::to_string(record.currentline);
            output += ':';
         }
         output += " in ";
         output += _function_name(L, record);
         if (record.istailcall)
            output += "\n\t(...tail calls...)";
      }
      lua_settop(L, top);
      lua_pushstring(L, output.c_str());
      return;
   }
}