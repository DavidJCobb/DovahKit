
# `api_helpers::native_list`

Metaprogramming helpers for defining a Dovahscript API that wraps a native list (i.e. a sequential, resizable list of elements with contiguous indices starting from 0 in C++).

Subclass `member_function_spec` and override/define fields as instructed in the code comments. Then, invoke `define_metatable` template don your spec class.

## Lua-facing API

### `list:clear()`
Available if the list allows removals. Deletes all items from the list.

### `list:index_of(v)`
Available for most lists.[^index-of-availability] Returns the one-based index of `v` within the list. Returns `nil` if `v` is not in the list.

May warn or error if `v` is not a legal value for the list. (As of this writing, this is not consistent. Some internal implementations use a custom check; others use the same underlying functionality to load the passed-in argument as when overwriting list items.)

For lists of sub-objects, where those objects can be constructed from/overwritten with a Lua table, this function *does not* handle table-type arguments; it will only compare exact userdata, and return `nil` for anything else.

[^index-of-availability]: Whether this is available depends on the list's native implementation i.e. whether the member function spec provides what the default `index_of_value` implementation `requires`. As of this writing, the requirements are: that the list's size be computable by calling `size()` on the wrapped list (or list*s*, if the stoarge is bifurcated); and that the value type be either sub-objects, or accessible via a provided `pull_value` function.

### `list:insert(...)`
Available if the list allows overwriting values. This can be invoked with the following signatures:

* **`list:insert()`\:** Inserts a default-constructed value at the end of the list.
* **`list:insert(v)`\:** Presuming `v` is (or can be used to construct) a value that can be stored in the list, appends `v` to the end of the list.
* **`list:insert(i)`\:** Inserts a default-constructed value at one-based index `i`.
* **`list:insert(i, v)`\:** Presuming `v` is (or can be used to construct) a value that can be stored in the list, inserts `v` at one-based index `i`.

The overloads that take a list index are only available if the list is not a list of numbers. The overloads that take a value to insert are only available for those native lists which support passing one.

### `list:remove(i)`
Available if the list allows removals. Removes the value at one-based index `i`.

### `#list`
Returns the list's length.

### `list[i] = v`
Available if the list allows overwriting values. Overwrites the value at one-based index `i`.
