
# How to expose form data to Lua

**Table of contents:**
* [Exposing basic form properties](./#Exposing-basic-form-properties)
  * [Defining an interface to expose to Lua](./#Defining-an-interface-to-expose-to-Lua)
    * Create the interface
    * Write code to set up its Lua-side class it at run-time
    * Write code so that forms returned to Lua are wrapped with the interface
  * [Defining the contents of the interface](./#Defining-the-contents-of-the-interface)
    * [Exposing form properties](./#Exposing-form-properties)
* [Exposing simple sub-objects](./#Exposing-simple-sub-objects)
  * Create a unique eight-CC for member access to the sub-object
  * Create the sub-object's interface
  * Write code to set up its Lua-side class it at run-time
  * Add a getter to the parent object
* [Exposing collections as Lua properties](./#Exposing-collections-as-Lua-properties)
  * [Indexed collections of scalar values](./#Indexed-collections-of-scalar-values)
    * Create the collection interface
    * Add a getter to the parent object
    * Write code to register the collection interface
  * [Indexed collections of sub-objects](./#Indexed-collections-of-sub-objects)
    * Handle insertions/removals causing existing items' indices to shift
  * [Collections with non-sequential integer keys](./#Collections-with-non-sequential-integer-keys) (e.g. quest aliases looked up by ID)
  * [Collections supporting access via string keys](./#Collections-supporting-access-via-string-keys) (e.g. quest aliases looked up by name)

## Exposing basic form properties

The simplest case is when exposing a form that has simple scalar-type accessors: no sub-objects; no internal arrays or other lists.

### Defining an interface to expose to Lua

Begin by creating a header resembling the following, in the `dovahscript/wrappers/form/` folder. This is the header for the Voicetype form type. The key thing we're doing here is defining a struct that will never be instantiated: a subclass of `wrapper_metatable` which defines some static members identifying the superclass(es), unique metatable key, class name, and the contents of the Lua interface we're defining.

```c++
#pragma once
#include "./form.h"

namespace dovah::loaded_forms {
   class Voicetype;
}

namespace dovahscript::wrappers {
   struct voicetype : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.voicetype";
      static constexpr const char*   class_name      = "voicetype";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Voicetype;
   };
}
```

<dl>
   <dt>class name</dt>
      <dd><p>A string used to identify this typename when reporting some errors in Lua.</p></dd>
   <dt>metatable key</dt>
      <dd><p>A unique string identifying the metatable for this class. When we wish to create instances of this wrapper &mdash; when we wish to expose a Voicetype form to Lua through this interface &mdash; we'll: create a Lua userdata whose native (C) data identifies the voicetype form; and then use this key to retrieve the metatable and apply that to the userdata so that Lua can actually call functions, access properties, et cetera, on the userdata.</p></dd>
</dl>

We also need to register the interface. Add an `#include` to `dovahscript/wrappers/form/_all.h`. Then, open `dovahscript/wrappers/set_up_all.cpp` and add the following line to `set_up_all_native_wrappers`:

```c++
define_wrapper_metatable<voicetype>(L);
```

Finally, we need to make sure that when a form of a given type is passed from native code to Lua, it actually uses this wrapper. Go to `dovahscript/push_native_object.cpp`, and add this line to the `form_classes` array:

```c++
std::pair{ dovah::form_type::voicetype, dovahscript::wrappers::voicetype::metatable_key },
```

### Defining the contents of the interface

Next, it's time to create the methods, getters, and setters. Create a CPP file with the following contents, just to start with.

```c++
#include "./voicetype.h"
#include "helpers/lua/error.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/core/subsystems/userdata.h"
#include "dovahscript/wrapper.h"

#include "dovah/forms/Voicetype.h"

namespace {
   using namespace dovahscript;
   using cls          = wrappers::voicetype;
   using wrapped_type = cls::wrapped_type;
   
   namespace _methods {
   }
   namespace _getters {
   }
   namespace _setters {
   }
}

namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = no_functions;
   /*static*/ cls::method_list_t cls::metatable_setters = no_functions;
}
```

What we need to do now is write the C(++) functions that will be exposed to Lua as methods, getters, and setters on any userdata using this interface. Let's start with a simple scalar property. Inside of the `_getters` namespace, add this function:

```c++
int allow_default_dialogue(lua_State* L) {
   auto& self = get_wrapper_for_thiscall<cls>(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   lua_pushboolean(L, (form->voicetype_flags & wrapped_type::voicetype_flag::allow_default_dialogue));
   return 1;
}
```

The first and only argument to the getter is the userdata which wraps the native object. The `get_wrapper_for_thiscall` function does type-checking, raises errors as necessary, and otherwise retrieves the `wrapper` instance. The `get_loaded_form_data` function on the wrapper loads the full data of the form in question and, if it's of the desired type, returns a pointer to that data.

Next, inside of the `_setters` namespace, add this function:

```c++
int allow_default_dialogue(lua_State* L) {
   core::subsystems::permissions::verify_form_write_permissions();
   
   auto& self = get_wrapper_for_thiscall<cls>(L);
   luaL_argcheck(L, lua_isboolean(L, 2), 2, "boolean expected");
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;

   self.before_edit();
   cobb::edit_bit(form->voicetype_flags, wrapped_type::voicetype_flag::allow_default_dialogue, lua_toboolean(L, 2));
   self.after_edit();
   return 0;
}
```

The general pattern for setters is that we begin by checking whether the script is allowed to modify loaded game data. (Dovahscript had early, unrealized, plans to allow scripts to optionally declare that they don't modify game data; this would've affected the messaging shown to users when they forcibly terminate a running script.) Then, we verify that the arguments passed in are of the right type, and that the wrapped form is actually reachable.[^lua-lifetime-issues] Finally, we call functions on the wrapper before and after editing, and we make our changes. These changes generally *should not fail*. The before/after functions send the `formModificationImminent` and `formModified` signals to the rest of the UI, and these signals should be paired.

The last thing we need to do to finish the interface is add these two functions to our lists of getters and setters:

```c++
namespace dovahscript::wrappers {
   /*static*/ cls::method_list_t cls ::metatable_methods = no_functions;
   
   /*static*/ cls::method_list_t cls::metatable_getters = {
      { "allows_default_dialogue", &_getters::allow_default_dialogue },
   };
   /*static*/ cls::method_list_t cls::metatable_setters = {
      { "allows_default_dialogue", &_setters::allow_default_dialogue },
   };
}
```

[^lua-lifetime-issues]: One of the problems we contend with is that we're exposing, to Lua, wrappers for native objects whose lifetimes are not under Lua's control. This means that when such a native object is deleted, the wrapper that we've given to Lua still exists &mdash; there's no way to make a userdata pretend to be `nil`, nor a practical way to find and nil out all variables referring to a given userdata. We therefore need our functions to validate that they're being called on a wrapper whose wrapped object still exists.

#### Exposing form properties

Returning a form to Lua works like this:

```c++
int field_name(lua_State* L) {
   auto& self = get_wrapper_for_thiscall<cls>(L);
   if (!self.stub)
      return 0;

   dovah::form_stub* some_form = /* ... */;
   return push_native_object(some_form);
}
```

Accepting a form via a setter works like this (using an ActorBase as the value type here):

```c++
int field_name(lua_State* L) {
   core::subsystems::permissions::verify_form_write_permissions();
   
   auto& self  = get_wrapper_for_thiscall<cls>(L);
   auto* form  = self.get_loaded_form_data<wrapped_type>();
   auto* value = pull_form_stub_argument(L, 2, dovah::form_type::actor_base);
   if (!form)
      return 0;

   self.before_edit();
   form->some_field.set(*form, value);
   self.after_edit();
   return 0;
}
```

## Exposing simple sub-objects

To expose a sub-object, we need to do two things:

* Define a Lua interface for the sub-object, as above.
* Define an eight-CC for this particular sub-object.

The directional material on Static forms is one example. Currently, the convention for sub-objects on specific form types is to define the eight-CC in the same file as the form wrapper:

```diff
#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Static;
}

+namespace dovahscript::wrapper_part_types {
+   inline constexpr cobb::eight_cc static_directional_material = "StatDMat";
+}

namespace dovahscript::wrappers {
   struct statik : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.static";
      static constexpr const char*   class_name      = "static";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Static;
   };
}
```

Next, we define the wrapper metatable, generally in `dovahscript/wrappers/form/form_type_name_here/`. An example:

```c++
#pragma once
#include "../static.h"

namespace dovah::loaded_forms {
   class Static;
}

namespace dovahscript::wrappers {
   struct static_directional_material : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.static_directional_material";
      static constexpr const char*   class_name      = "static_directional_material";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using form_type = dovah::loaded_forms::Static;
   };
}
```

You'll need to register the metatable alongside the Static metatable. Indentation is generally used to indicate sub-objects within this function.

```c++
define_wrapper_metatable<statik>(L);
   define_wrapper_metatable<static_directional_material>(L);
```

Finally, we need a member function on the Static interface that returns this sub-object. This is where we use the previously created eight-CC. Our native wrappers use eight-CCs (and sometimes indices) to identify member access on native objects.

```c++
int directional_material(lua_State* L) {
   auto& self = get_wrapper_for_thiscall<cls>(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   wrapper out = self;
   out.append_part(wrapper_part_types::static_directional_material);
   return core::subsystems::userdata::get().push(L, out, wrappers::static_directional_material::metatable_key);
}
```

## Exposing collections as Lua properties

Dovahscript has a system for "collections" which automates a lot of the boilerplate related to certain container-type sub-objects &mdash; specifically, arrays and resizable vectors. The system can also be used for anything that is, conceptually, a map of strings to values, but only if the values are stored with consistent ordering internally (e.g. a `std::vector` of things that have unique names).

Each collection needs two things: an eight-CC; and a `collection_definition_params` object, which defines the member functions and other behavior for the collection's Lua interface.

### Indexed collections of scalar values

Let's use the collection of a FormList's entries as an example. The FormList interface header contains an eight-CC, `'FormList'`, for the entries. The header for the collection's definition is fairly simple:

```c++
#pragma once
#include "dovahscript/core/collections.h"

namespace dovahscript::wrappers::collections {
   extern const collection_definition_params formlist_entries;
}
```

The implementation file is more complex. Let's start by looking *just* at the object definition above:

```c++
namespace dovahscript::wrappers::collections {
   extern const collection_definition_params formlist_entries = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
      .get_collection_length  = &get_collection_length,
      .lookup_item_by_index   = &lookup_item_by_index,
      .member_function_insert = &member_function_insert,
      .member_function_remove = &member_function_remove,
      .set_item               = &set_item,
   };
}
```

The `get_collection_length` and `lookup_item_by_index` functions are mandatory for indexed access: the former implements the length operator (i.e. `#coll`) and the latter, indexed read-access (i.e. `v = coll[n]`). The other functions are optional: you can have the collection expose `:insert()` and `:remove()` functions to Lua, and you can make it possible to overwrite collection items (e.g. `coll[n] = v`).

Read-access is easy to implement, but remember that Lua uses one-indexed lists, whereas native code is zero-indexed.

```c++
int get_collection_length(lua_State* L) {
   auto& self = get_collection_wrapper(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   lua_pushinteger(L, form->contents.size());
   return 1;
}
int lookup_item_by_index(lua_State* L) {
   auto& self = get_collection_wrapper(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   auto  i    = lua_tointeger(L, 2);
   auto& list = form->contents;
   if (i > list.size() || i <= 0)
      return 0;
   --i;
   return push_native_object(list[i]);
}
```

When implementing your "insert" function, the first argument is the collection's userdata, and any arguments past that are arguments passed to `insert` when it was invoked. Your "remove" implementation works similarly. Both should check permissions as form data setters do (see above).

Once your collection is implemented, you will of course need to make it possible for people to actually access it. For FormList entries, the getter looks like this:

```c++
int entries(lua_State* L) {
   auto& self = get_wrapper_for_thiscall<cls>(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   wrapper out = self;
   out.append_part(wrapper_part_types::formlist_entries);
   out.is_collection = true;
   return core::subsystems::userdata::get().push(L, out, wrappers::collections::formlist_entries.registry_key);
}
```

This is similar to exposing a sub-object, except that we mark the wrapper as a collection.

Finally, you need to register the metatable. There's some helper functionality we can use: we can define an "extra class setup" function on the `FormList` wrapper definition, and then have that implementation call the function that turns collection definition params into a Lua-side class:

```diff
#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class FormList;
}

+namespace dovahscript::wrapper_part_types {
+   inline constexpr cobb::eight_cc formlist_entries = "FormList";
+}

namespace dovahscript::wrappers {
   struct formlist : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.formlist";
      static constexpr const char*   class_name      = "formlist";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::FormList;

+      static constexpr bool has_extra_class_setup = true;
+      static void extra_class_setup(lua_State* L);
   };
}
```

And the implementation:

```c++
/*static*/ void cls::extra_class_setup(lua_State* L) {
   define_collection_metatable(L, collections::formlist_entries);
}
```


### Indexed collections of sub-objects

This is similar to dealing with scalars. The main differences lie in how your collection returns an item, and how it inserts and removes items.

Returning an item generally looks like this, in yor `lookup_item_by_index` function:

```c++
wrapper out = self;
assert(out.is_collection);
assert(out.parts[0].signature == wrapper_part_types::topic_info_response);
out.into_collection(i);
return core::subsystems::userdata::get().push(L, out, wrappers::topic_info_response::metatable_key);
```

When removing an item, you have to signal to our Lua userdata subsystem that the item has been destroyed and its next-siblings have shifted up an index. You do this by creating a `wrapper` to the removed item, and then invoking a function on the userdata subsystem. An example, from TopicInfo's collection of responses:

```c++
self.before_edit();
{
   list[i].clear(*form);
   list.erase(list.begin() + i);
}
self.after_edit();
{
   wrapper to_remove = self;
   to_remove.into_collection(i);
   core::subsystems::userdata::get().remove_from_sequential_collection(to_remove);
}
```

A similar signal is needed for insertions, passing the collection wrapper and the zero-indexed position at which a new item is being inserted. You can skip this if you're appending to the end.

```c++
core::subsystems::userdata::get().insert_into_sequential_collection(self, insert_at);
```


### Collections with non-sequential integer keys

This is used for looking up a quest's aliases by their ID. When returning a collection item, you just have to flag the `wrapper` as non-sequential. An example, for quest aliases:

```diff
wrapper out = collection;
assert(out.is_collection);
assert(out.parts[0].signature == wrapper_part_types::quest_alias_by_id || out.parts[0].signature == wrapper_part_types::quest_alias);
out.parts[0].signature = wrapper_part_types::quest_alias_by_id;
out.into_collection(alias->id);
+out.last_part().noncontiguous = true;
```


### Collections supporting access via string keys

First, mark the collection as accepting string keys, and give it functions related to string access:

```diff
namespace dovahscript::wrappers::collections {
   extern const collection_definition_params quest_alias_set = {
      .registry_key           = collection_metatable_key,
      .garbage_collection     = &wrapper::__gc,
      //
+      .get_all_item_names     = &get_all_item_names,
      .get_collection_length  = &get_collection_length,
+      .items_are_named        = true,
+      .lookup_item_by_name    = &lookup_item_by_name,
      .lookup_item_by_index   = &lookup_item_by_index,
   };
}
```

The `get_all_item_names` function needs to return a Lua array (i.e. a plain table) containing all of the string keys. This is used to power `next` and friends.

```c++
int get_all_item_names(lua_State* L) {
   auto& self = get_collection_wrapper(L);
   auto* form = self.get_loaded_form_data<wrapped_type>();
   if (!form)
      return 0;
   auto& list = form->aliases;
   //
   lua_createtable(L, 0, list.size());
   auto index_tbl = lua_gettop(L);
   //
   for (const auto* alias : list) {
      lua_pushboolean(L, true);
      lua_setfield(L, index_tbl, alias->name.c_str());
   }
   return 1;
}
```

The `lookup_item_by_name` function just needs to look the item up by the passed-in name, returning the item or (if nothing is found) `nil`.
