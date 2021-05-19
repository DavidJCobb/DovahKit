#include "discontiguous_list.h"
#include <cassert>

/*

   For documentation purposes, here is the code for luaL_ref and luaL_unref translated 
   from C to Lua for clarity:

      function luaL_ref(tbl, val)
         if val == nil then
            return nil
         end
         local ref = math.tointeger(tonumber(tbl[0]) or 0)
         if ref ~= 0 then
            tbl[0] = tbl[ref]
         else
            ref = #tbl + 1
         end
         tbl[ref] = val
         return ref
      end

      function luaL_unref(tbl, ref)
         if ref >= 0 then
            tbl[ref] = tbl[0]
            tbl[0] = ref
         end
      end

   So what we're doing is, we're using tbl[0] as a linked list of empty slot indices in 
   tbl, storing those indices within tbl itself. So a table that might normally look like 
   this

      0 == nil
      1 == object
      2 == object
      3 == nil
      4 == nil
      5 == object
      6 == nil

   would instead look like this, presuming the mid-table nil values were removed in order 
   of their keys:

      0 == 3
      1 == object
      2 == object
      3 == 4
      4 == 6
      5 == object
      6 == nil

   And this forms the linked list: [3, 4, 6]. Because tbl[6] == nil, that is the list's 
   end node.

   If the values weren't removed in order, the table might instead look like this:

      0 == 4
      1 == object
      2 == object
      3 == 6
      4 == 3
      5 == object
      6 == nil

   That forms the linked list [4, 3, 6]. Again, tbl[6] is the list's end node.

   This presents a problem when trying to iterate over the table's keys.

*/

namespace cobb::lua::discontiguous_list {
   extern int insert(lua_State* L, int table_pos) {
      //
      // function insert(tbl, val)
      //    local idx = #tbl + 1
      //    tbl[idx] = val
      //    return idx
      // end
      //
      table_pos = lua_absindex(L, table_pos);
      int index = lua_rawlen(L, table_pos);
      ++index;
      lua_rawseti(L, table_pos, index);
      return index;
   }

   extern void remove(lua_State* L, int table_pos, int index) {
      //
      // function remove(tbl, idx)
      //    if idx <= 0 then
      //       return
      //    end
      //    tbl[idx] = nil
      // end
      //
      if (index <= 0) {
         return;
      }
      table_pos = lua_absindex(L, table_pos);
      lua_pushnil(L);
      lua_rawseti(L, table_pos, index);
   }
}