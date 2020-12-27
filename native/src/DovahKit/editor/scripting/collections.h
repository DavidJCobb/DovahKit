#pragma once
#include "util.h"

namespace editor_script {
   extern void define_collection_metatable(
      lua_State* L,
      const char* registry_key,
      lua_CFunction garbage_collection,    // __gc metamethod (optional)
      bool items_are_named,
      lua_CFunction get_collection_length, // args: wrapper;        return: number
      lua_CFunction lookup_item_by_name,   // args: wrapper, name;  return: wrapper or nil
      lua_CFunction lookup_item_by_index,  // args: wrapper, index; return: wrapper or nil
      lua_CFunction get_all_item_names     // args: wrapper;        return: table of names
   );
}