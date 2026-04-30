#pragma once
#include "../../lua.h"

namespace dovahscript {
   struct collection_definition_params {
      const char*   registry_key       = nullptr;
      lua_CFunction garbage_collection = nullptr; // __gc metamethod (optional)
      //
      // The fields below should be kept alphabetized  in order to aid with the use of designated 
      // initializers in C++20 and onward.
      //
      // The fields whose names  are prefixed with "member_function_" represent  member functions 
      // that can appear  on a collection, i.e.  "member_function_foo" is  "my_collection:foo()". 
      // These are all optional, and are only written to the Lua table if the collection does not 
      // support named elements. Lua tables do not  distinguish between function and non-function 
      // members, which means that in a collection that  supports named elements, member function 
      // names would risk conflicting with collection element names.
      //

      // Receives the  wrapper as an argument, and  returns an array of names.  Required by named 
      // collections in order to support pairs().
      lua_CFunction get_all_item_names     = nullptr;

      // Receives the wrapper  as an argument, and  returns the number  of elements inside of the 
      // collection. Optional; used to provide the __len metamethod.
      lua_CFunction get_collection_length  = nullptr;

      // Enables named elements.
      const bool    items_are_named        = false;

      // Receives the wrapper and a name as an argument, and returns a collection element or nil. 
      // Required if named elements are enabled.
      lua_CFunction lookup_item_by_name    = nullptr;

      // Receives the wrapper  and an index as an  argument, and returns a  collection element or 
      // nil. Optional. An ipairs iterator will start  at 1 and stop on the first nil, so this is 
      // not suitable for collections with noncontiguous indices, or for collections that require 
      // zero-based indices. The index received will always  be a number, but may not actually be 
      // stored as one (e.g. "5").
      lua_CFunction lookup_item_by_index   = nullptr;

      // Optional member  function named "insert." The  first argument is the  wrapper; all other 
      // arguments are those passed by the script. Should generally mimic table.insert; this will 
      // make it  functionally redundant  with  table.insert,  except  that this function  can be 
      // optimized based on the inner workings of the wrapped container.
      lua_CFunction member_function_insert = nullptr;

      // Optional member  function named "remove." The  first argument is the  wrapper; all other 
      // arguments are those passed by the script.  Should generally accept an element name or an 
      // element index, and remove the targeted element  from the collection, adjusting the other 
      // elements' indices to avoid leaving a gap in the list.
      lua_CFunction member_function_remove = nullptr;

      // Receives the wrapper, a key, and a value as arguments. Returns nothing. Optional. Should 
      // generally modify  the collection by inserting, replacing, or removing  an element in the 
      // manner a script author would expect.
      lua_CFunction set_item               = nullptr;
   };

   extern void define_collection_metatable(lua_State* L, const collection_definition_params&);
}