
# Helper library

I have a general-purpose C++ library in the top-level "helpers" folder &mdash; just any random odds and ends that I end up needing, whether for metaprogramming or in general. I should reorganize this, and give the folder a better name.

## Organization

### Folders

* `lu/arrays/`
* `lu/ascii/`
  * `lu::ascii::equals`
  * `lu::ascii::equals_i` (case-insensitive)
* `lu/bit/`
  * `lu::edit_bit`
* `lu/concepts/` &mdash; does not create a nested namespace
* `lu/containers/` &mdash; does not create a nested namespace
  * `lu::resizable_grid`
* `lu/math/`
  * `lu/math/geometry/`
  * `lu/math/types/` &mdash; does not create a nested namespace
    * `lu::math::axis_angle`
    * `lu::math::euler`
    * `lu::math::matrix`
    * `lu::math::quaternion`
    * `lu::math::rotation_matrix`
    * `lu::math::vector3` with a shorthand typedef `lu::vec3`
* `lu/memory/`
  * `./multiheap.h`
* `lu/optionals/`
* `lu/scope_guards/`
* `lu/strings/`
* `lu/tuples/`
* `lu/type_containers/`
  * `lu::type_containers::class_array`
  * `lu::type_containers::class_to_value_map`
* `lu/type_traits/` &mdash; does not create a nested namespace
  * `./function_traits.h`
* `lu/vectors/`

## Individual fixes to specific classes

* `cobb::vector3<T>` uses a `throw` statement when its `operator[]` is run in constexpr, to reject out-of-bounds accesses. This statement triggers compiler warnings when the operator is invoked; I should suppress those warnings. (Not doing that now because the header is used in enough places to have a fairly wide blast radius.)
