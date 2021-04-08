#pragma once
#include "util.h"

//
// Here's a common problem that Lua doesn't really offer a good solution for: what happens if 
// you need to make natively-managed objects  accessible to a script, but have those objects' 
// lifetimes managed entirely by native code? You can't just have the objects be deleted when 
// they go out of scope in the script, but you can offer the script a "delete" API.
//
// There's no way to find all references to a userdata and force them to nil remotely, nor is 
// there a way to make a userdata impersonate nil or otherwise test as falsy. This means that 
// all you can really do is put the userdata in an unusable state, and make it possible for a 
// script to detect that you've done this.
//
// The (zombify_userdata) function provided below is designed to do the former:  when given a 
// userdata,  it will create a "zombie class"  hierarchy parallel to the userdata's own class 
// hierarchy (refer to classes.h).  Each zombie class is tagged with a unique sentinel, which 
// in turn allows the (userdata_is_zombie) function to identify any zombie instance.
//

namespace editor_script {
   constexpr const char* dead_class_metatable_storage = "__dead_classes";    // registry key for the table in which zombie class metatables are stored
   constexpr const char* zombie_sentinel_key          = "__zombie_sentinel"; // registry key for a sentinel userdata used to mark zombie classes

   extern luastackchange_t zombify_userdata(lua_State*); // call via lua_call, not directly
   extern bool userdata_is_zombie(lua_State* L, int stack_pos);
}