
# Form sub-object property helpers

We have a "wrapper" system for exposing access to top-level native objects: forms, Qt widgets, and so on. This system also supports accessing "sub-objects" within a native object, such as a response in a Topic Info form.

The sub-object property helpers here exist to strip away a lot of the code duplication involved in writing APIs for sub-objects. You can define a single list of property definitions, and then from that, you can generate per-field [gs]etter, `assign` and `overwrite_with` member functions for the sub-object, and setters for a sub-object's containing collection (e.g. `my_topic_info.responses[3] = { ... }`).

## High-level overview

### Usage

The basic approach is as follows:

* You define a `constexpr std::tuple` of property definitions.

* You can invoke `subobject_property_helpers::verify_table<wrapped_type, true, subobject_properties>(L, table_pos)` to verify that a Lua value (in state `L` at stack position `table_pos`) is a valid table to apply to the sub-object.

  * `wrapped_type` is the sub-object type.

  * The boolean parameter indicates whether `nil` values are always allowed. It should be `true` for "assign," since the intended semantics there are that nil values mean "don't change the existing value of tihs field," and `false` for "overwrite."

  * `subobject_properties` is a non-type template parameter: a reference to your tuple of property definitions.

* You can then invoke `subobject_property_helpers::apply_table<wrapped_type, true, subobject_properties>(L, table_pos, dst_form, dst_subobject);` to actually apply the table.

  * Template parameters are the same as `verify_table`.

* You can mark your sub-object's wrapper as requiring extra class setup. Within the setup function, invoke `subobject_property_helpers::setup_extra_getters_and_setters<cls, &_unwrap_and_require, subobject_properties>(L);`.

  * `cls` is the struct that represents the wrapper and its metatable.
  
  * `_unwrap_and_require` is a function which, given a `lua_State*`, will check whether argument 1 is [a Lua userdata which wraps] a sub-object. It'll either raise an error, or return a reference to the sub-object.

  * As above, `subobject_properties` is a non-type template parameter: a reference to your tuple of property definitions.

### Property definitions

Each property definition consists of four core elements:

* An accessor function which, given a sub-object, returns a mutable reference to the desired field in that sub-object.

* A check function which, given a Lua state and stack position, checks whether the value at that stack position is valid for assigning to the field.

* A pull function which, given a Lua state and stack position, extracts the property value from that stack position, converting it as necessary to a type that can be assigned to the field.

* A push function which, given a Lua state and a const reference to the sub-object field, will push that field's current value onto the Lua stack.

Metaprogramming is then used to invoke these functions as appropriate, building [gs]etters and apply-table behaviors on top of them.

Some optional behaviors are available as well:

* You can define a default value for a property, or specify that the property should treat `nil` as "unchanged" even during `subobject:overwrite_with({ ... })`. These two options influence what happens when you overwrite the sub-object with a table that doesn't define the property:

  * If `nil` is treated as "unchanged," then the sub-object field is left unchanged.
  * If a default value is provided, then the sub-object is set to that value.
  * Otherwise, the sub-object is reset to whatever value would result from default-construction.

* A late-check function can be defined for use within setters, to double-check whether the pulled value is valid not merely on its own, but in the context of the sub-object and its containing form. The normal check function is invoked before the sub-object is even accessed; the late-check function is invoked after the sub-object becomes available but before the value is written.
  
  One example use case is for Topic Info responses: each response must have a different `unique_id`. The check function tests whether a value supplied by Lua is an integer in the valid range, and warns if that integer is zero. The late-check function tests whether the response's containing form has any other responses that are already using the value as their own unique ID, and raises an error if so.

## Defining properties

Helper types and functions are provided for common property types. The general convention is `subobject_property_helpers::foo_property<params>::define("name", accessor)`. The available helpers as of this writing are:

* `subobject_property_helpers::enumeration_property<Mapping>::define("name", accessor)`
  * The mapping must be a `std::array` of `std::pair<enumeration, std::string_view>>`. The limitations of non-type template parameters are such that this must be defined as a separate constant with its own identifier.
* `subobject_property_helpers::form_property<AllowedFormType, AllowNone>::define("name", accessor)`
  * `AllowedFormType` can be a single `dovah::form_type` value or a `std::array` thereof. If you don't want to constrain the allowed form types, you can use `dovah::form_type::none`.
  * `AllowNone` determines whether passing `nil` is legal or an error. It defaults to `true`.
* `subobject_property_helpers::integer_property<Min, Max>::define("name", accessor)`
* `subobject_property_helpers::integer_property<Min, Max>::define("name", accessor, default_value)`
* `subobject_property_helpers::localized_string_property::define("name", accessor)`
* `subobject_property_helpers::std_string_property::define("name", accessor)`

When custom behavior is needed, you can instantiate a property definition yourself, though this is more cumbersome due to the constraints on the types involved:

```c++
subobject_property_helpers::property_definition{
   .name   = "unique_id",
   .access = [](wrapped_type& src) [[msvc::forceinline]] -> auto& { return src.id; },
   .check  = [](lua_State* L, int pos) -> std::string_view {
      if (!lua_isinteger(L, pos))
         return "integer expected";
      auto v = lua_tointeger(L, pos);
      if (v < 0 || v > form_type::max_available_response_ids)
         return "value is out of bounds";
      if (v == 0)
         lua_warning(L, "a response ID of 0 is invalid (it is the sentinel used while recording lines in the Creation Kit)", 0);
      return {};
   },
   .pull = &subobject_property_helpers::pull::integer,
   .push = [](lua_State* L, const decltype(wrapped_type::id)& value) { lua_pushinteger(L, value); },

   .late_check = [](lua_State* L, const dovah::loaded_forms::Form& form, const wrapped_type& subobject, const lua_Integer& v) -> void {
      if (subobject.id == v)
         return;
      assert(form.stub.form_type == dovah::form_type::topic_info);
      const auto& info = static_cast<const dovah::loaded_forms::TopicInfo&>(form);
      for (const auto& resp : info.responses) {
         if (resp.id == v && &resp != &subobject) {
            luaL_error(L, "the provided response ID (%u) is already in use by another response on this TopicInfo", v);
         }
      }
   },

   .treat_nil_as_unchanged = true,
},
```

