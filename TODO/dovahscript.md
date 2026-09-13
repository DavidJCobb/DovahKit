
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
  
  
## Outstanding issues

### Deletion of forms can break collection wrappers

Suppose a Dovahscript collection wraps a key/value map where the keys, and where the map itself is implemented as a `std::vector` of entries. Thus, the containing form for this map will react to the deletion of any form *K* by removing entries from this `std::vector` whose keys are *K*.

Dovahscript has no way to be notified about this, nor any way to signal to any given collection wrapper that *K* is being deleted (so we can't even give collections some entry point whereby they could check for and handle this case).

Dovahscript's `coordinator` subsystem is notified about the script-initiated deletion of forms, so in theory it could handle this kind of thing. Currently, it doesn't do anything (other than just track what forms are supposed ot be deleted, and verify that it's received signals for them; but it doesn't *do* anything in response to those signals, so this is all just dead code).

### Some API helpers can't cope with non-editable forms

Right now, there's a "native list" API helper that uses tons of template metaprogramming to allow us to quickly and conveniently create API bindings any list-like sub-object of a wrappable object. However, there's no way for us to get these API helpers to properly call `dovahscript::api_helpers::fail_if_form_cannot_be_edited(L, self)`. The same issue exists for the "sub-object property helpers."

Currently, the only non-editable forms are none-stubs, forms with `form_type::none` (e.g. the PapyrusPersistenceForm), and the PlayerRef. However, I want to add calls to the "fail if cannot be edited" function to all APIs that write to form data, as a form of future-proofing. I explicitly can't do this for anything wrapped as a "native list."

Refactors of Dovahscript should have separate means of unwrapping forms/form data, depending on whether we're unwrapping for a read or for a write. Then, we wouldn't even need all this ceremony; the API itself could just call the single "unwrap for write" function, and that would run all needed checks to see if the form still exists (i.e. not a zombie wrapper) and is legal to write to. (Unfortunately, the current design just has a single "get wrapper for thiscall" function for both reads and writes, and a single "get loaded form data" function for both reads and writes.)


## Needed improvements

### Implement and allow `require`

We don't currently expose `require` to script packages. Instead, script files must be listed in a `manifest.xml` file; this determines the order in which scripts are loaded or executed. This... kinda sucks? It forces a script package developer to manually manage file execution order, instead of allowing files to `require` their dependencies.

It has an additional drawback. I'd actually love to be able to expose extensions to the Lua standard library, e.g. `extable.join(a, b)` or `extable.reserve(table, arr_count, name_count)`, but I'd prefer for those singletons to be explicitly requested by a script via e.g. `local extable = require "@dovah/extable"`. Since we don't currently expose `require`, that's... just not something we can offer.

Of course, Lua's built-in `package` library, which exports `require`, has a lot of configuration details that need to be seen to, so we can ensure that users can't load arbitrary Lua files outside of the script package, load standard library functions we don't want to expose, load DLLs, et cetera.


### Better error reporting

Lua APIs like `luaL_argerror`, and functions in my helper library which wrap it (like `cobb::lua::argerror`), rely on hardcoded logic to identify the names of faulting functions. This logic can't be extended e.g. to properly report the names of setters in the Dovahscript API. The result is error messages like these --

```
bad argument #2 to '?' (unrecognized run-on type)
stack traceback:
	[C]: in setter: condition.run_on
	userscript:8: in main chunk
```

-- wherein Dovahscript's custom stack-trace logic properly identifies the name of the faulting function (`setter: condition.run_on`), but the initial error message lists the function as `'?'`.

Dovahscript should expose, and its APIs should use, a collection of common functions for reporting these styles of errors, e.g. `dovahscript::throw_because::bad_setter_rhs` or `dovahscript::throw_because::bad_argument`. These functions should share the same logic used within our stack-trace code, to be able to properly identify the throwing API function. The use of `luaL_argerror`, `luaL_argcheck`, and friends within Dovahscript's APIs should be deprecated.

An alternative implementation to sharing logic with the stack-trace code -- perhaps more reliable -- would be to have a hidden global table such that `FUNCTION_DESCRIPTORS[func]` is a table like the following:

```lua
{
   is_member_of = "condition",
   name         = "run_on",
   type         = "setter",
   is_static    = false,       -- not a static member function
}
```

#### Debugging Lua errors

Consider this a pipe dream.

It'd be nice if we could offer a debugger for uncaught Lua errors. It wouldn't allow running Lua code, continuing script execution, etc., but it'd be cool if we could allow users to inspect the values of variables at each call stack frame.

This is harder than it sounds, of course. The "obvious" approach would be to avoid using Lua's API in favor of inspecting its internal data (essentially doing what my Lua Natvis already does, but in C++), so we can allow the user to see a frozen snapshot of all Lua call stack frames. However, that approach will fail completely when dealing with native objects; unless we implement two sets of getters (one within Lua, and one external to Lua which returns a `std::any` wrapping Lua-suitable values), native objects can only be inspected (in "Lua-compatible" form) by invoking their getters, and we can't know to a certainty that that would be side-effect-free (including possible side effects on the overall Lua state).


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

Additionally, it'd be nice if the collection internals could handle more of the nuances of collections. In particular, when dealing with a collection of sub-objects, it'd be nice if the "remove" function could signal success, with the internals then killing any extant wrappers for the removed sub-object (with all of the needed side-effects, e.g. shifting indices of any next-sibling sub-objects if the collection uses contiguous indices).

#### Collection improvements

Right now, Dovahscript uses a common abstraction, "collections," for all of the following:

* Native arrays of fixed length (e.g. `shout.words`)
* Native vectors (e.g. `land_texture.grasses`)
  * ...including cases where a bifurcated list needs to be presented as joined (e.g. `topic_info.conditions`)
* Native key/value maps, when the keys are exclusively integers and/or strings (e.g. `quest.aliases` and `quest.aliases_by_id`)

We should give each of these their own independently programmed abstraction.

#### Overwriting sub-objects wholesale

Right now, it's not possible to do something like this:

```lua
my_shout.words = {
   {
      word  = foo,
      spell = bar,
   },
   {
      word  = foo,
      spell = bar,
   },
   {
      word  = foo,
      spell = bar,
   },
}
```

In order to implement this in Dovahscript as of this writing (i.e. before any of the refactors described above), we'd have to manually write a setter that validates each of the passed-in tables. This is the case both for writing to `my_shout.words` and for writing to any elements *in* `words` i.e. `my_shout.words[2] = { ... }`. Of course, I'd prefer for most if not all sub-objects to be overwriteable in this way, and handwriting this logic for every possible sub-object isn't a solution that would scale well.

However, with the refactor described above, we sort of get this for free. Template metaprogramming can generate default validation for most properties (e.g. ensuring that you don't write out-of-bounds values into an `int32_t`, or ensuring that you don't write forms of the wrong type into a `form_use` that specifies its allowed form types). Custom validation functions can handle the rest. This means that all we have to do to support overwriting a non-collection sub-object is:

* Error if the RHS table has any properties not present (or not writeable) on the sub-object.
* For each writeable property on the sub-object, error if the corresponding value (or nil) on the RHS table fails validation.
* Otherwise, invoke each property's setter individually, *or* invoke an "overwrite all at once" function if we've handwritten one for that sub-object.

For overwriting collections, the logic above is performed per collection element, with collection-level behaviors differing depending on whether the collection is fixed-size, et cetera.


### Wrappers have a flawed design

The design for wrappers outlined [above](./#Wrappers) has some limitations:

* **Wrappers can only represent member access by unsigned numeric index, or by predefined property names (internally represented as eight-CCs).** Arbitrary property names aren't possible, though I can't think of any cases where sub-objects are identified solely by user-defined strings. What's more problematic is that key/value maps aren't representable in this system; for example, the NavMeshInfoMap singleton form stores a mapping of Navmesh forms to NavmeshInfo structs (this mapping comprimises the bulk of the form's data and is what the form is named after), and our wrapper system can't grant access to that because it can't use form stub pointers as keys.

* **It's not clear how to represent collections of collections.** If the only distinction between `foo.bar` and `foo.bar[5]` is a single flag set across the entire wrapper, then how do we distinguish `foo.bar[5][7]` from either of those? This has come up when dealing with NavMeshInfoMap form data: the form stores a list of precomputed paths, and each precomputed path is just a list of navmesh forms; thus `navi.precomputed_paths[i][j]` should yield a navmesh form.

I need to rethink wrappers.


### Collection wrappers need the ability to have custom member functions

We mainly need this for TopicInfos' condition lists, which are bifurcated due to jank with how the game loads `INFO` records. The lists are split into "locked" conditions which cannot be edited in any way, and "normal" conditions which are alterable. It'd be nice if we could query, on the list, how many of the conditions are "locked."


### Better access to UI models

Right now, Dovahscript makes it possible for Lua to refer to a "flat" model's rows, columns, and cells, a way that accounts for the insertion or deletion of rows (i.e. if Lua refers to row #5, and we delete row #3, then Lua will still refer to the same row even though that row is now #4).

However, I don't believe I ever implemented this functionality for trees, so we can't offer things like treeviews. Such an implementation would be fairly similar, except that deleting a model item would require us to find and sever any Lua references to child or descendant items.

What's more: Lua can't attach arbitrary data to model items. There's no system whereby we can bind data to e.g. `(Qt::ItemDataRole)(Qt::UserRole + 1000)`. This makes some scripted tasks kind of painful, and it means that widgets like treeviews would have extremely limited utility. If we wanted to implement this, we'd need a system whereby the Qt model item stores a handle to Lua data, and that data is refcounted. We'd need two tables:

* When you try to add Lua data to a model item, we store that data in a strong (i.e. non-weak) table and reserve a handle for it.

* When we actually get around to creating the model item natively, we store a reference to the Lua data in a weak table, store the refcounting handle in the Qt model, and then remove the Lua data from the strong table.

This order of operations should ensure that the data isn't dropped in transit.

