
# Dovahscript

## Current design

### Wrappers

Native objects are exposed to Lua via <dfn>wrappers</dfn>, and the wrapped interfaces are defined using some template metaprogramming.

A wrapper consists of a root native object, followed by zero or more <dfn>parts</dfn>. Each part represents member access deeper into the native object and its subobjects; parts have an eight-CC and an unsigned index. The wrapper as a whole has an "is collection" flag: the sole difference between the two wrappers produced by `some_shout.words` versus `some_shout.words[2]` is the `is_collection` flag being set on the latter.

This of course has some limitations, discussed below.

Non-"collection" wrappers, i.e. wrappers for top-level native objects and for "struct-like" sub-objects, have their interfaces defined via subclasses of `wrapper_metatable`. The `wrapper_metatable` type and its subclasses are never instantiated; they exclusively define `static` members and are treated as compile-time values when metaprogramming. Specifically, they define:

* A list of superclasses from which a given wrapper interface inherits.
* A metatable key, i.e. a unique string ID for the interface.
* A class name, for use in error reporting and some other cases.
* A list of instance member functions.
* A list of instance getter functions.
* A list of instance setter functions.
* Optionally, an "extra class setup" function that can be used to alter the Lua-side class object, its getters, or its setters, or to register relevant collection metatables.

"Collection" wrappers have their interfaces defined via instances of `collection_definition_params`. Collections can support element access by string name or numeric index, though under the hood, wrappers always identify collection elements by index. Collection parameters are passed to `define_collection_metatable` to build a collection class within Lua. Collections can define the following parameters:

* A bool indicating whether collection elements can be accessed by name.
* For named access
  * A Lua C-function which takes a collection wrapper and returns an array containing the names of all collection elements. This is needed for Lua `pairs()` iteration.
  * A Lua C-function which takes a collection wrapper and a name, and returns the element matching that name, if any.
* For indexed access
  * A Lua C-function which takes a collection wrapper and returns the number of elements. This is needed for Lua `#length` operators.
  * A Lua C-function which takes a collection wrapper and an index, and returns the element at that index, if any.
* Either-or
  * Optionally, an "insert item" function whose arguments should mimic those of `table.insert`.
  * Optionally, a "remove item" member function.
  * Optionally, a "set item by key" function.
  

## Needed improvements

### Easier script bindings for form data

Writing these is extremely unpleasant and entails a massive amount of boilerplate. I *desperately* need to find some way to reduce that boilerplate via template metaprogramming.

Native objects are exposed to Lua via wrappers, and the wrapper interfaces are defined using some template metaprogramming; however, the parameters and parts of those interfaces are... painfully manual.

It'd be nice if we could define property access by just specifying the property name and an accessor function which returns a reference to the target field, with template metaprogramming then generating getters and setters from that. For most record flags, it's even simpler: we'd just pass the property name and the flag bitmask. For accessing nested collections, we could maybe template on a collection interface. Using Navmesh forms as an example, it'd look something like this:

```c++
using loaded_form_type = dovah::loaded_forms::Navmesh;

/*static*/ form_property_list cls::properties = {
   record_flag_property("auto_generated",     loaded_form_type::form_flag::auto_generated),
   record_flag_property("initially_disabled", loaded_form_type::form_flag::initially_disabled),
   
   collection_property<collections::navmesh::door_links>("door_links"),
   collection_property<collections::navmesh::edge_links>("edge_links"),
   collection_property<collections::navmesh::triangles>("triangles"),
   collection_property<collections::navmesh::vertices>("vertices"),
   
   // For scalar-field properties, template metaprogramming would take care of 
   // validating the "this" argument and extracting a reference to loaded-form 
   // data from it. We'd return a reference to the target field, and then the 
   // template machinery would perform get/set operations as appropriate.
   //
   // There is one caveat: properties may need to validate their inputs, so we 
   // would want to be able to optionally pass some third argument to serve as 
   // a validator. This argument could be a list of allowed form types, or a 
   // non-capturing lambda that takes a `lua_State*` argument and is expected 
   // to validate the top of the stack (while leaving the stack unchanged at 
   // exit).
   property(
      "version",
      [](loaded_form_type& self) -> auto& {
         return self.geometry.version;
      }
   ),
};
```

Similarly, it'd be nice if collections could be defined more easily. Using form lists as an example, here's the kind of syntax we should aim for:

```c++
using loaded_form_type = dovah::loaded_forms::FormList;

namespace wrappers::forms::form_lists {

   // NOTE:
   // `form_list_collection` here refers to a collection whose entries are form stubs. 
   // it is not specific to "form list" forms, despite those being the topic of this 
   // example.
   
   extern const collection_interface list_item_collection = form_list_collection<loaded_form_type>{
      .accessor = [](loaded_form_type& form) -> auto& {
         return form.entries;
      },
      .allow_none_entries = true,
      .can_insert         = true,
      .can_remove         = true,
      
      //
      // There are cases where we may want to expose something to Lua as a list of 
      // forms when in reality, it's a list of structs whose only meaningful data is 
      // form stub pointers. For example, door links in NavmeshInfo structs consist 
      // of a CRC (identifying the struct type, because Bethesda got weird with this) 
      // and a load door ref. We'd want to expose that to Lua as just a collection of 
      // load door refs.
      //
      // To allow for this, we could perhaps have something like:
      //
      //    .item_accessor = [](navmesh_info::door_link& item) -> form_use& {
      //       return item.door;
      //    },
      //
      // That lambda, when present, could be invoked to remap both read and write 
      // access to list items.
      //
   };
}
```

Collections of structs could perhaps look like this:

```c++
using loaded_form_type  = dovah::loaded_forms::Shout;
using item_wrapper_type = wrappers::forms::shouts::word;

namespace wrappers::forms::shouts {
   extern const collection_interface word_collection = struct_list_collection<loaded_form_type, item_wrapper_type>{
      .accessor = [](loaded_form_type& form) {
         return form.words;
      },
      //
      // No need for "can insert," "can remove," etc.. The template machinery 
      // should detect that the accessor returns a reference to an array, and 
      // should realize that the collection is therefore fixed-size.
      //
   };
}
```

Named collections:

```c++
using loaded_form_type  = dovah::loaded_forms::Quest;
using item_wrapper_type = wrappers::forms::quests::alias;

namespace wrappers::forms::quests {
   extern const collection_interface alias_by_name_collection = struct_list_collection<loaded_form_type, item_wrapper_type>{
      .accessor = [](loaded_form_type& form) {
         return form.aliases;
      },
      
      // By default, you can always access by index. If we wanted a "by names only" 
      // collection, then we'd do this:
      .allow_indexed_access = false,
      
      // Optional. If provided, then named access becomes allowed. Templates set up 
      // all of the needed functions for this -- looking up items by name, returning 
      // an array!table of all names, setting/inserting/removing by name, et cetera.
      .get_item_name = [](const typename item_wrapper_type::wrapped_item_type& subobject) {
         return std::string_view(subobject.name);
      },
   };
}
```

### Wrappers have a flawed design

The design for wrappers outlined [above](./#Wrappers) has some limitations:

* **Wrappers can only represent member access by unsigned numeric index, or by predefined property names (internally represented as eight-CCs).** Arbitrary property names aren't possible, though I can't think of any cases where sub-objects are identified solely by user-defined strings. What's more problematic is that key/value maps aren't representable in this system; for example, the NavMeshInfoMap singleton form stores a mapping of Navmesh forms to NavmeshInfo structs (this mapping comprimises the bulk of the form's data and is what the form is named after), and our wrapper system can't grant access to that because it can't use form stub pointers as keys.

* **It's not clear how to represent collections of collections.** If the only distinction between `foo.bar` and `foo.bar[5]` is a single flag set across the entire wrapper, then how do we distinguish `foo.bar[5][7]` from either of those? This has come up when dealing with NavMeshInfoMap form data: the form stores a list of precomputed paths, and each precomputed path is just a list of navmesh forms; thus `navi.precomputed_paths[i][j]` should yield a navmesh form.

I need to rethink wrappers.
