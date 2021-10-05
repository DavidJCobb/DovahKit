#include "classes.h"
#include <cassert>
#include "../../helpers/lua/error.h"
#include "../../helpers/lua/metamethod_names.h"
#include "../../helpers/lua/raw.h"
#include "../../helpers/lua/setfuncs.h"
#include "../../helpers/lua/set_top_on_exit.h"

//
// Our class system in Lua allows for multiple inheritance by mimicking the way that 
// MSVC lays out the RTTI for polymorphic classes in C++. Each class has a flat list 
// called the "__classlist", which contains all of the class's ancestor classes, and 
// ends with the class itself. There is  also a "__superclasses" member which serves 
// to reproduce the class tree, in case that is ever useful. As such, member lookups 
// and cast operations need only walk the flat list, while more intensive operations 
// e.g. for debugging could potentially walk the full tree.
// 
// The most recently specified parent  classes take priority over the least recently 
// specified parent classes, when doing member lookups.
// 
// Diamond inheritance is not supported. You can control whether we go to the effort 
// of checking for it and asserting that it doesn't occur; if we don't check for it, 
// or if you're compiling on a configuration where asserts are omitted, then it will 
// simply result in undefined behavior.
//

// Config
namespace {
   static constexpr bool check_for_diamond_inheritance = true;
}

// Helpers
namespace {
   bool _should_skip_name(lua_State* L, int index) {
      index = lua_absindex(L, index);
      if (lua_isstring(L, index)) {
         auto nk = lua_tostring(L, index);
         if (cobb::lua::is_metamethod_name(nk))
            return true;
         else if (strcmp(nk, "__getters") == 0)
            return true;
         else if (strcmp(nk, "__setters") == 0)
            return true;
         else if (strcmp(nk, "__superclasses") == 0)
            return true;
         else if (strcmp(nk, "__classlist") == 0)
            return true;
         else if (strcmp(nk, "__name") == 0)
            return true;
         else if (strcmp(nk, "__custom_class_internals") == 0)
            return true;
      }
      return false;
   }

   int _push_zero(lua_State* L) {
      lua_pushinteger(L, 0);
      return 1;
   }
   
   void _forward_metamethod_to_subclass(lua_State* L, const char* metamethod, int superPos, int subPos) {
      superPos = lua_absindex(L, superPos);
      subPos   = lua_absindex(L, subPos);
      //
      // Don't overwrite the metamethod if it already exists on the subclass metatable:
      //
      lua_pushstring(L, metamethod);
      if (lua_rawget(L, subPos) != LUA_TNIL) {
         lua_pop(L, 1);
         return;
      }
      lua_pop(L, 1);
      //
      // Copy the metamethod from the superclass table to the subclass table, if it 
      // exists on the former:
      //
      lua_pushstring(L, metamethod); // STACK: [metamethod, ...]
      lua_pushstring(L, metamethod); // STACK: [metamethod, metamethod, ...]
      lua_rawget    (L, superPos);   // STACK: [super[metamethod], metamethod, ...]
      if (!lua_isnil(L, -1))
         lua_rawset(L, subPos); // STACK: [...]
      else
         lua_pop(L, 2);
   }
}

// Pairs iterator code
namespace {
   namespace _pairs_iterator { // code for __pairs iterators
      constexpr const char* metatable_key = "-cobb-class-helpers:pairs-iterator";
      //
      static int __call(lua_State* L) {
         /*
            function __call(self, t, k)
               if getmetatable(t) ~= self.meta then
                  return
               end
               if k then
                  self.seen[k] = true
               end
               local v     = nil
               local list  = rawget(self.meta, "__classlist")
               local count = #list
               for i = count - self.done, 1, -1 do
                  local cls = list[i]
                  if not self.getters then
                     k, v = next(cls, k)
                     while _should_skip_name(k) or self.seen[k] do
                        k, v = next(cls, k)
                     end
                     if k then
                        return k, v
                     end
                     self.getters = true
                  end
                  if self.getters then
                     cls = cls.__getters
                     --
                     k, v = next(cls, k)
                     while _should_skip_name(k) or self.seen[k] do
                        k, v = next(cls, k)
                     end
                     if k then
                        if v then
                           v = (v)(t)
                        end
                        return k, v
                     end
                     --
                     -- No getters or all getters iterated; move on to
                     -- next class.
                     --
                     self.done    = self.done + 1
                     self.getters = false
                  end
               end
               return nil, nil -- implicit
            end
         */
         constexpr auto index_self = 1;
         constexpr auto index_t    = 2;
         constexpr auto index_k    = 3;
         constexpr auto index_meta = 4;
         constexpr auto index_seen = 5;
         constexpr auto index_list = 6;
         constexpr auto index_cls  = 7;
         constexpr auto index_nk   = 8;
         constexpr auto index_nv   = 9;
         //
         lua_getmetatable(L, index_t);
         cobb::lua::rawgetfield(L, index_self, "seen");
         {  // Verify that the target table's metatable has not been replaced.
            cobb::lua::rawgetfield(L, index_self, "meta");
            if (!lua_rawequal(L, -1, index_meta))
               return 0;
            lua_pop(L, 1);
         }
         if (!lua_isnoneornil(L, index_k)) {
            // Update the "seen" table.
            lua_pushvalue  (L, index_k);
            lua_pushboolean(L, true);
            lua_rawset     (L, index_seen);
         }
         //
         cobb::lua::rawgetfield(L, index_meta, "__classlist");
         assert(lua_gettop(L) == index_list);
         auto count = lua_rawlen(L, index_list);
         for (decltype(count) i = count; i > 0; --i) {
            lua_rawgeti(L, index_list, i);
            //
            cobb::lua::rawgetfield(L, index_self, "getters");
            bool getters = lua_toboolean(L, -1);
            lua_pop(L, 1);
            if (!getters) {
               while (lua_next(L, index_cls) != 0) {
                  if (!_should_skip_name(L, index_nk)) {
                     cobb::lua::rawgetvalue(L, index_seen, index_nk);
                     bool seen = lua_toboolean(L, -1);
                     lua_pop(L, 1);
                     if (!seen)
                        return 2;
                  }
                  lua_settop(L, index_nk);
               }
               getters = true;
               lua_pushboolean(L, true);
               lua_setfield(L, index_self, "getters");
            }
            if (getters) {
               cobb::lua::rawgetfield(L, index_cls, "__getters");
               lua_replace(L, index_cls);
               //
               while (lua_next(L, index_cls) != 0) {
                  if (!_should_skip_name(L, index_nk)) {
                     cobb::lua::rawgetvalue(L, index_seen, index_nk);
                     bool seen = lua_toboolean(L, -1);
                     lua_pop(L, 1);
                     if (!seen) {
                        //
                        // Execute the getter.
                        //
                        lua_pushvalue(L, index_t);
                        lua_call(L, 1, 1);
                        //
                        return 2;
                     }
                  }
                  lua_settop(L, index_nk);
               }
               //
               lua_pushboolean(L, false);
               lua_setfield(L, index_self, "getters");
            }
         }
         return 0;
      }
      //
      static void define_metatable(lua_State* L) {
         auto start = lua_gettop(L);
         bool defined = luaL_getmetatable(L, metatable_key) == LUA_TTABLE;
         lua_settop(L, start);
         if (defined)
            return;
         luaL_newmetatable(L, metatable_key);
         auto index_mt = start + 1;
         //
         lua_pushcfunction(L, &__call);
         lua_setfield(L, index_mt, "__call");
         //
         lua_settop(L, start);
      }
   }
}

// Metamethod bodies
namespace {
   namespace _metamethods {
      static int __index(lua_State* L) {
         /*
            function __index(t, k)
               if _should_skip_name(k) then
                  return nil
               end
               local meta  = getmetatable(t)
               local list  = rawget(meta, "__classlist")
               local count = #list
               for i = count, 1, -1 do
                  local cls = list[i]
                  local a   = rawget(cls, k)
                  if a then
                     return a
                  end
                  a = rawget(cls, "__getters")
                  if a then
                     a = rawget(a, k)
                     if a then
                        return a(t)
                     end
                  end
               end
            end
         */
         auto index_table = 1;
         auto index_key   = 2;
         if (_should_skip_name(L, index_key))
            return 0;
         if (!lua_getmetatable(L, index_table)) // STACK: - [ t, k, meta ] +
            return 0;
         auto index_meta = 3;
         cobb::lua::rawgetfield(L, index_meta, "__classlist");
         auto index_list = 4;
         //
         #if _DEBUG
            const char* __key = lua_tostring(L, index_key);
         #endif
         //
         auto count = lua_rawlen(L, index_list);
         for (decltype(count) i = count; i > 0; --i) {
            lua_settop (L, index_list);
            lua_rawgeti(L, index_list, i);
            //
            cobb::lua::rawgetvalue(L, -1, index_key);
            if (!lua_isnoneornil(L, -1))
               return 1;
            lua_pop(L, 1);
            //
            cobb::lua::rawgetfield(L, -1, "__getters");
            //
            // Stack:
            //    -1 | 6: list[i].__getters
            //    -2 | 5: list[i]
            //    -3 | 4: list
            //    -4 | 3: meta
            //    -5 | 2: k
            //    -6 | 1: t
            //
            if (lua_istable(L, -1)) {
               cobb::lua::rawgetvalue(L, -1, index_key);
               if (!lua_isnoneornil(L, -1)) {
                  //
                  // Stack:
                  //    -1 | 7: list[i].__getters[k]
                  //    -2 | 6: list[i].__getters
                  //    -3 | 5: list[i]
                  //    -4 | 4: list
                  //    -5 | 3: meta
                  //    -6 | 2: k
                  //    -7 | 1: t
                  //
                  // Desired stack for getter-function call:
                  //    -1 | 3: t
                  //    -2 | 2: list[i].__getters[k]
                  //    -3 | 1: t // intentional leftover, queryable through Lua debug APIs while the function runs
                  //
                  // STACK: - [ t, k, meta, list, list[i], list[i].__getters[k] ] +
                  lua_copy(L,  1, 2);  // STACK: - [ t, t, meta,                 list, list[i], list[i].__getters[k] ]+
                  lua_copy(L, -1, 3);  // STACK: - [ t, t, list[i].__getters[k], list, list[i], list[i].__getters[k] ] +
                  lua_settop(L, 3);    // STACK: - [ t, t, list[i].__getters[k] ] +
                  lua_rotate(L, 2, 1); // STACK: - [ t, list[i].__getters[k], t ] +
                  lua_call(L, 1, 1);   // STACK: - [ t, list[i].__getters[k](t) ] +
                  //
                  // Quick explanation for my own reference, since basically only one page on the entire Internet 
                  // has documented this and it's not the Lua manual: given the stack
                  //
                  //     A B C D E
                  //     1 2 3 4 5
                  //
                  // A call to lua_rotate(L, 3, 1) will produce:
                  //
                  //     A B | E C D
                  //     1 2 | 3 4 5
                  //
                  // Positive offsets rotate right; negative, left.
                  //
                  return 1;
               }
            }
            // Don't bother popping at the end of the loop; we settop at the start.
         }
         return 0;
      }
      static int __newindex(lua_State* L) {
         /*
            function __newindex(t, k, v)
               if _should_skip_name(k) then
                  return nil
               end
               local has_getter = false
               local meta  = getmetatable(t)
               local list  = rawget(meta, "__classlist")
               local count = #list
               for i = count, 1, -1 do
                  local cls = list[i]
                  local a   = rawget(cls, "__setters")
                  if a then
                     a = a[k]
                     if a then
                        a(t, v)
                        return
                     end
                  end
                  if not has_getter then
                     a = rawget(meta, "__getters")
                     if a and a[k] then
                        has_getter = true
                     end
                  end
               end
               local classname = getmetatable(t).__name or "?"
               if has_getter then
                  error(string.format("DovahKIt does not allow you to assign to property '%s' on class %s", k, classname))
               end
               error(string.format("class %s does not offer a property named '%s'", classname, k))
            end
         */
         constexpr int index_table = 1;
         constexpr int index_key   = 2;
         constexpr int index_value = 3;
         if (_should_skip_name(L, index_key))
            return 0;
         if (!lua_getmetatable(L, index_table)) // STACK: - [ t, k, meta ] +
            return 0;
         constexpr int index_meta = 4;
         cobb::lua::rawgetfield(L, index_meta, "__classlist");
         constexpr int index_list = 5;
         //
         #if _DEBUG
            const char* __key = lua_tostring(L, index_key);
         #endif
         //
         bool has_getter = false;
         auto count      = lua_rawlen(L, index_list);
         for (decltype(count) i = count; i > 0; --i) {
            lua_settop (L, index_list);
            lua_rawgeti(L, index_list, i);
            constexpr int index_cls = 6;
            //
            cobb::lua::rawgetfield(L, index_cls, "__setters");
            if (lua_istable(L, -1)) {
               cobb::lua::rawgetvalue(L, -1, index_key);
               if (lua_isfunction(L, -1)) {
                  //
                  // Stack:
                  //    -1 | 8: list[i].__setters[k]
                  //    -2 | 7: list[i].__setters
                  //    -3 | 6: list[i]
                  //    -4 | 5: list
                  //    -5 | 4: meta
                  //    -6 | 3: v
                  //    -7 | 2: k
                  //    -8 | 1: t
                  // 
                  // Desired stack for call:
                  //    -1 | 4: v
                  //    -2 | 3: t
                  //    -3 | 2: list[i].__setters[k]
                  //    -4 | 1: t // intentional leftover, queryable through Lua debug APIs while the function runs
                  //
                  lua_copy(L,  3, 4);
                  lua_copy(L,  1, 3);
                  lua_copy(L, -1, 2);
                  lua_settop(L, 4);
                  lua_call(L, 2, 0);
                  return 0;
               }
               lua_pop(L, 1);
            }
            lua_pop(L, 1);
            //
            if (!has_getter) {
               cobb::lua::rawgetfield(L, index_cls, "__getters");
               if (lua_istable(L, -1)) {
                  cobb::lua::rawgetvalue(L, -1, index_key);
                  has_getter = !lua_isnoneornil(L, -1);
                  lua_pop(L, 1);
               }
               lua_pop(L, 1);
            }
         }
         //
         const char* key       = lua_tostring(L, index_key);
         const char* classname = "?";
         lua_settop      (L, index_value);
         lua_getmetatable(L, index_table); // STACK: - [ t, k, v, meta ] +
         if (cobb::lua::rawgetfield(L, index_meta, "__name") == LUA_TSTRING) {
            classname = lua_tostring(L, -1);
         }
         if (has_getter)
            cobb::lua::error(L, "DovahKit does not allow you to assign to property '%s' on class %s", key, classname);
         cobb::lua::error(L, "class %s does not offer a property named '%s'", classname, key);
      }
      static int __pairs(lua_State* L) {
         /*
            function __pairs(t)
               local iter = {
                  meta    = getmetatable(t),
                  seen    = {},
                  done    = 0,
                  getters = false,
               }
               setmetatable(iter, REGISTRY[PAIRS_ITERATORS_METATABLE_KEY])
               return iter, t, nil
            end
         */
         lua_settop(L, 1);
         lua_checkstack(L, 2);
         lua_createtable  (L, 0, 4); // index 2 (iter)
         luaL_getmetatable(L, _pairs_iterator::metatable_key);
         lua_setmetatable (L, 2);
         //
         lua_getmetatable(L, 1);
         lua_setfield    (L, 2, "meta");
         //
         lua_createtable(L, 0, 0);
         lua_setfield   (L, 2, "seen");
         //
         lua_pushinteger(L, 0);
         lua_setfield(L, 2, "done");
         //
         lua_pushboolean(L, false);
         lua_setfield   (L, 2, "getters");
         //
         lua_pushvalue(L, 1);
         lua_pushnil(L);
         return 3;
      }
   }
}

namespace dovahscript::classes {
   //
   // If the element at (stack_pos) is an instance of the class described by the 
   // (class_internal_name), then this returns the userdata pointer; otherwise, it 
   // returns nullptr. This only works for userdata, not for tables.
   //
   [[nodiscard]] extern void* cast_to_class(lua_State* L, int stack_pos, const char* class_metatable_key) {
      /*
         function cast_to_class(t, class_metatable_key)
            if type(t) ~= "userdata" then
               return nil
            end
            local desired = REGISTRY[class_metatable_key]
            local meta    = getmetatable(t)
            local list    = meta.__classlist
            if type(list) ~= "table" then
               return nil
            end
            for i = 1, #list do
               if list[i] == desired then
                  return t
               end
            end
            return nil
         end
      */
      auto  guard = cobb::lua::set_top_on_exit(L, lua_gettop(L));
      void* ud    = lua_touserdata(L, stack_pos);
      if (!ud)
         return nullptr;
      //
      luaL_getmetatable(L, class_metatable_key);
      if (!lua_getmetatable(L, stack_pos))
         return nullptr;
      lua_pushstring(L, "__classlist");
      lua_rawget    (L, -2);
      if (!lua_istable(L, -1))
         return nullptr;
      //
      // Stack:
      //    -1: list
      //    -2: meta
      //    -3: desired
      //
      auto len = lua_rawlen(L, -1);
      for (decltype(len) i = 1; i <= len; ++i) {
         lua_rawgeti(L, -1, i);
         //
         // Stack:
         //    -1: list[i]
         //    -2: list
         //    -3: meta
         //    -4: desired
         //
         if (lua_rawequal(L, -1, -4))
            return ud;
         lua_pop(L, 1);
      }
      return nullptr;
   }

   //
   // Same as (cast_to_class), except that it does not allow subclasses of the 
   // desired class; the userdata on the stack must be an exact match. Suitable 
   // for classes that you don't intend to ever subclass. This only works for 
   // userdata, not for tables.
   //
   [[nodiscard]] extern void* cast_to_exact_class(lua_State* L, int stack_pos, const char* class_metatable_key) {
      //
      // LUA:
      //    if not userdata then
      //       return nil
      //    end
      //    local instanceMeta = getmetatable(userdata)
      //    local classMeta    = getmetatable(classKey)
      //    if classMeta ~= instanceMeta do
      //       return nil
      //    end
      //    return userdata
      //
      lua_checkstack(L, 2);
      auto  top = lua_gettop(L);
      void* ud  = lua_touserdata(L, stack_pos);
      if (!ud)
         return nullptr;
      if (!lua_getmetatable(L, stack_pos))
         return nullptr;
      luaL_getmetatable(L, class_metatable_key);
      // STACK: [classMeta, instanceMeta, ...]
      if (!lua_rawequal(L, -1, -2)) {
         lua_settop(L, top);
         return nullptr;
      }
      lua_settop(L, top);
      return ud;
   }

   //
   // Similar to (cast_to_class), but returns a bool, and can be used on both 
   // userdata and normal tables. This is useful for things like utility classes 
   // backed by native code: scripts can be given the ability to instantiate 
   // normal tables using the class metatables, and this function can then be 
   // used by native code to validate these tables.
   //
   [[nodiscard]] extern bool check_for_class(lua_State* L, int stack_pos, const char* class_metatable_key) {
      /*
         function cast_to_class(t, class_metatable_key)
            local desired = REGISTRY[class_metatable_key]
            local meta    = getmetatable(t)
            local list    = meta.__classlist
            if type(list) ~= "table" then
               return false
            end
            for i = 1, #list do
               if list[i] == desired then
                  return true
               end
            end
            return false
         end
      */
      auto guard = cobb::lua::set_top_on_exit(L, lua_gettop(L));
      //
      luaL_getmetatable(L, class_metatable_key);
      if (!lua_getmetatable(L, stack_pos))
         return false;
      lua_pushstring(L, "__classlist");
      lua_rawget    (L, -2);
      if (!lua_istable(L, -1))
         return false;
      //
      // Stack:
      //    -1: list
      //    -2: meta
      //    -3: desired
      //
      auto len = lua_rawlen(L, -1);
      for (decltype(len) i = 1; i <= len; ++i) {
         lua_rawgeti(L, -1, i);
         //
         // Stack:
         //    -1: list[i]
         //    -2: list
         //    -3: meta
         //    -4: desired
         //
         if (lua_rawequal(L, -1, -4))
            return true;
         lua_pop(L, 1);
      }
      return false;
   }
   
   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(
      lua_State* L,
      const char* class_metatable_key,
      const std::initializer_list<const char*>& superclass_metatable_keys,
      const std::initializer_list<luaL_Reg>& methods,
      const std::initializer_list<luaL_Reg>& getters,
      const std::initializer_list<luaL_Reg>& setters,
      const char* class_name // __name value for class metatable; defaults to (class_metatable_key) if not specified
   ) {
      /*
         _define_pairs_metatable() -- lazy-create the pairs iterator class
         if super_metatable_keys then
            for _, v in ipairs(super_metatable_keys) do
               assert(class_metatable_key ~= v, "A superclass and subclass can't use the same registry key name.")
            end
         end
         local meta = {}
         if class_name then
            meta.__name = class_name
         else
            meta.__name = class_metatable_key
         end
         REGISTRY[class_metatable_key] = meta
         --
         meta.__index    = __index
         meta.__newindex = __newindex
         meta.__pairs    = __pairs
         meta.__len      = _push_zero -- #{} == 0, but #userdata == error by default
         --
         meta.__classlist = {} -- list including all ancestor classes and itself
         if super_metatable_keys then
            local count = #super_metatable_keys
            if count > 0 then
               meta.__superclasses = {}
               --
               do
                  local flat_count = 1
                  local seen       = {}
                  for i = 1, count do
                     local name  = super_metatable_keys[i]
                     local super = REGISTRY[name]
                     if super == meta then
                        error("A class cannot be its own superclass.")
                     end
                     supers[i] = super
                     --
                     local sc = #super.__classlist
                     for j = 1, sc do
                        local cls = super.__classlist[j]
                        if seen[cls] then
                           error(string.format("New class '%s' has diamond inheritance of '%s'.", meta.__name, cls.__name))
                        end
                        meta.__classlist[flat_count] = cls
                        seen[cls]  = true
                        flat_count = flat_count + 1
                     end
                     --
                     meta.__superclasses[i] = super
                  end
               end
               --
               -- Iterate the list in reverse order, so that when we forward the
               -- metamethods from superclasses to the subclass, the later super-
               -- classes in the list take precedence over the earlier ones.
               --
               for i = count, 1, -1 do
                  local super = meta.__superclasses[i]
                  _forward_metamethod_to_subclass("__tostring", super, meta)
                  _forward_metamethod_to_subclass("__close",    super, meta)
                  _forward_metamethod_to_subclass("__gc",       super, meta)
               end
            end
         end
         meta.__classlist[#meta.__classlist + 1] = meta
         --
         if methods then
            for k, v in pairs(methods) do
               meta[k] = v
            end
         end
         if getters then
            meta.__getters = {}
            for k, v in pairs(getters) do
               meta.__getters[k] = v
            end
         end
         if setters then
            meta.__setters = {}
            for k, v in pairs(setters) do
               meta.__setters[k] = v
            end
         end
      */
      _pairs_iterator::define_metatable(L);
      lua_checkstack(L, 3);
      //
      int created  = luaL_newmetatable(L, class_metatable_key); // STACK: [newmeta]
      int index_mt = lua_gettop(L);
      assert(created && "This Lua class is already defined!");
      //
      if (class_name) {
         lua_pushstring(L, "__name");
         lua_pushstring(L, class_name);
         lua_rawset    (L, index_mt);
      }
      //
      cobb::lua::setfuncs(L, {
         { "__index",    &_metamethods::__index },
         { "__newindex", &_metamethods::__newindex },
         { "__pairs",    &_metamethods::__pairs },
         { "__len",      &_push_zero }, // #{} == 0, but #userdata == error; let's not.
      });
      //
      {
         lua_createtable(L, 1, 0);
         lua_pushstring (L, "__classlist");
         lua_pushvalue  (L, -2);
         lua_rawset     (L, index_mt);
         int index_cl = index_mt + 1;
         assert(lua_gettop(L) == index_cl);
         if (auto count = superclass_metatable_keys.size()) {
            lua_createtable(L, count, 0);
            lua_pushstring (L, "__superclasses");
            lua_pushvalue  (L, -2);
            lua_rawset     (L, index_mt);
            int index_tree = index_cl   + 1;
            int index_seen = index_tree + 1;
            assert(lua_gettop(L) == index_tree);
            if constexpr (check_for_diamond_inheritance) {
               lua_createtable(L, 1, 0);
               assert(index_seen == lua_gettop(L));
            }
            //
            int flat_count = 0;
            int tree_count = 0;
            for (auto* name : superclass_metatable_keys) {
               luaL_getmetatable(L, name);
               assert(lua_istable(L, -1) && "The desired superclass doesn't yet have a metatable set up. Are you setting up your classes in the wrong order?");
               //
               lua_pushstring(L, "__classlist");
               lua_rawget(L, -2);
               int  index_scl = lua_gettop(L); // superclass list
               auto sc_len    = lua_rawlen(L, -1);
               for (decltype(sc_len) i = 1; i <= sc_len; ++i) {
                  lua_rawgeti(L, index_scl, i);
                  if constexpr (check_for_diamond_inheritance) {
                     lua_pushvalue(L, -1);
                     lua_rawget   (L, index_seen);
                     assert(lua_isnoneornil(L, -1) && "Diamond inheritance is not allowed.");
                     lua_pop(L, 1);
                     //
                     lua_pushvalue  (L, -1);         // 
                     lua_pushboolean(L, true);       // 
                     lua_rawset     (L, index_seen); // seen[scl[i]] = true
                  }
                  lua_rawseti(L, index_cl, ++flat_count);
               }
               lua_pop(L, 1);
               //
               // Now, the superclass metatable is at the top of the stack. Let's write 
               // it into meta.__superclasses, which will also pop it from the stack.
               //
               lua_rawseti(L, index_tree, ++tree_count);
            }
            if constexpr (check_for_diamond_inheritance) {
               lua_pop(L, 1); // ditch the "seen" table.
            }
            //
            // Finally, we need to forward metamethods from the superclasses to the new 
            // subclass. The last-specified superclasses should take precedence, so we'll 
            // reverse-iterate the list.
            //
            assert(lua_gettop(L) == index_tree);
            for (int i = tree_count; i > 0; --i) {
               lua_rawgeti(L, index_tree, i);
               _forward_metamethod_to_subclass(L, "__tostring", -1, index_mt);
               _forward_metamethod_to_subclass(L, "__close",    -1, index_mt);
               _forward_metamethod_to_subclass(L, "__gc",       -1, index_mt);
               lua_pop(L, 1);
            }
            lua_pop(L, 1);
         }
         assert(lua_gettop(L) == index_cl);
         lua_pushvalue(L, index_mt);                              // 
         lua_rawseti  (L, index_cl, lua_rawlen(L, index_cl) + 1); // meta.__classlist[#meta.__classlist + 1] = meta
         //
         lua_pop(L, 1);
      }
      //
      if (methods.size()) {
         cobb::lua::setfuncs(L, methods); // import functions into the metatable
      }
      if (getters.size()) {
         lua_pushstring (L, "__getters");       // push 1
         lua_createtable(L, 0, getters.size()); // push 1
         cobb::lua::setfuncs(L, getters);       // push 0
         lua_settable(L, index_mt);             // pop  2
      }
      if (setters.size()) {
         lua_pushstring (L, "__setters");       // push 1
         lua_createtable(L, 0, setters.size()); // push 1
         cobb::lua::setfuncs(L, setters);       // push 0
         lua_settable(L, index_mt);             // pop  2
      }
      //
      lua_pop(L, 1); // pop metatable from the stack
   }

   //
   // Define a metatable for a class, and store the metatable in the registry using 
   // (class_internal_name) as the key name. You can specify the key name of a super-
   // class in (superclass_internal_name).
   //
   extern void define_class(
      lua_State* L,
      const char* class_metatable_key,
      const char* superclass_metatable_key,
      const std::initializer_list<luaL_Reg>& methods,
      const std::initializer_list<luaL_Reg>& getters,
      const std::initializer_list<luaL_Reg>& setters,
      const char* class_name // __name value for class metatable; defaults to (class_metatable_key) if not specified
   ) {
      if (superclass_metatable_key)
         define_class(L, class_metatable_key, { superclass_metatable_key }, methods, getters, setters, class_name);
      else
         define_class(L, class_metatable_key, {}, methods, getters, setters, class_name);
   }

   extern void extend_class(
      lua_State* L,
      const char* class_metatable_key,
      const std::vector<luaL_Reg>& methods,
      const std::vector<luaL_Reg>& getters,
      const std::vector<luaL_Reg>& setters
   ) {
      luaL_getmetatable(L, class_metatable_key);
      int index_mt = lua_gettop(L);
      if (lua_type(L, index_mt) != LUA_TTABLE) {
         lua_pop(L, 1);
         return;
      }
      //
      if (methods.size()) {
         cobb::lua::setfuncs(L, methods); // import functions into the metatable
      }
      if (getters.size()) {
         luaL_getsubtable(L, index_mt, "__getters");
         cobb::lua::setfuncs(L, getters);
         lua_pop(L, 1);
      }
      if (setters.size()) {
         luaL_getsubtable(L, index_mt, "__setters");
         cobb::lua::setfuncs(L, setters);
         lua_pop(L, 1);
      }
      //
      lua_pop(L, 1);
   }

   // Pushes the following tables to the Lua stack: the class metatable; the class's 
   // getter list; and the class's setter list. If either of the latter two lists is 
   // missing (which can happen if the class had no [gs]etters), they will be created. 
   // Returns the number of values pushed to the Lua stack (0 in case of failure).
   [[nodiscard]] extern int get_class_tables(
      lua_State* L,
      const char* class_metatable_key
   ) {
      lua_checkstack(L, 3);
      bool exists = luaL_getmetatable(L, class_metatable_key) == LUA_TTABLE;
      int  index  = lua_gettop(L);
      if (!exists) {
         lua_pop(L, 1);
         return 0;
      }
      luaL_getsubtable(L, index, "__getters");
      luaL_getsubtable(L, index, "__setters");
      return 3;
   }

   //
   // Check registry key (class_internal_name) for a class metatable.
   //
   [[nodiscard]] extern bool is_class_defined(lua_State* L, const char* class_metatable_key) {
      bool result = luaL_getmetatable(L, class_metatable_key) == LUA_TTABLE;
      lua_pop(L, 1);
      return result;
   }
}