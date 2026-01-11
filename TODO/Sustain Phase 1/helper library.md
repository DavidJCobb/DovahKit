
# Helper library

I have a general-purpose C++ library in the top-level "helpers" folder &mdash; just any random odds and ends that I end up needing, whether for metaprogramming or in general. I should reorganize this, and give the folder a better name.

## Organization

### Folders and files/declarations

* `lu/algorithms/` &mdash; not all content therein is in a nested namespace
  * `lu::sort_and_remember`
* `lu/arrays/`
* `lu/ascii/`
  * `lu::ascii::equals`
  * `lu::ascii::equals_i` (case-insensitive)
* `lu/bit/` &mdash; not all content therein is in a nested namespace
  * `lu::bitswap`
  * `lu::edit_bit`
  * `lu::bits::all_ones`
  * `lu::bits::replace_at_index`
  * `lu::bits::replace_most_significant`
* `lu/color/`
  * `lu::rgb_bytes`
  * `lu::rgb_floats`
  * `lu::rgba_bytes`
  * `lu::rgba_floats`
* `lu/concepts/` &mdash; does not create a nested namespace
* `lu/containers/` &mdash; does not create a nested namespace
  * `lu/containers/grids/` &mdash; does not create a nested namespace
    * `lu::fixed_grid`
      * `lu::corner_square_grid`
      * `lu::centered_square_grid`
    * `lu::resizable_grid`
  * `lu/containers/trees` &mdash; does not create a nested namespace
    * `lu::homogenous_tree<node_data_type>`: Tree of nodes with homogenous data. Offers member functions e.g. `lu::homogenous_tree<T>::node::contains(const node&)` for you.
  * `lu::bitfield_array`
  * `lu::small_vector`
* `lu/enums/` &mdash; does not create a nested namespace
  * `lu::enum_flags`
  * `lu::enum_in`
* `lu/math/`
  * `lu::pi`
  * `lu/math/geometry/`
  * `lu/math/rotation/`
    * `lu::degrees_to_radians`
    * `lu::degrees_to_radians_exact`
    * `lu::radians_to_degrees`
    * `lu::radians_to_degrees_exact`
  * `lu/math/types/` &mdash; does not create a nested namespace
    * `lu::math::axis_angle`
    * `lu::math::euler`
    * `lu::math::matrix`
    * `lu::math::quaternion`
    * `lu::math::rotation_matrix`
    * `lu::math::vector3` with a shorthand typedef `lu::vec3`
* `lu/memory/` &mdash; does not create a nested namespace
  * `lu::mapped_file`
  * `lu::multiheap`
* `lu/optionals/`
* `lu/scope_guards/`
* `lu/streams/` &mdash; does not create a nested namespace
  * Study the design of `std::format` to determine how to make these easily extensible.
  * Investigate whether we can make it possible to extend a subclass of a given stream, without extending every stream. This would be most useful for domain-specific streams, e.g. a subclass of `lu::bitreader` intended for specific data only.
  * `lu::bitreader`
  * `lu::bitwriter`
  * `lu::bytereader`
  * `lu::bytewriter`
* `lu/strings/`
  * `lu::strings::interned_table`
* `lu/tuples/`
* `lu/type_containers/`
  * `lu::type_containers::class_array`
  * `lu::type_containers::class_to_value_map`
* `lu/type_traits/` &mdash; does not create a nested namespace
  * `./function_traits.h`
* `lu/vectors/`
  * `lu::vectors::convert` (formerly `map_to_new_type`)
  * `lu::vectors::move_item_after_index`
  * `lu::vectors::move_item_before_index`
  * `lu::vectors::move_item_by_distance`
  * `lu::vectors::move_range_after_index`
  * `lu::vectors::move_range_before_index`
  * `lu::vectors::move_range_by_distance`
  * `lu::vectors::re_sort_item_within`
  * `lu::vectors::unordered_erase`
* `lu/windows/`
  * `./forward_declare_handles.h`
* `lu/`
  * `lu::bound_mem_fn` and `lu__bound_this_fn`
  * `lu::cpuinfo`
  * `lu::dynamic_exact_cast`
  * `lu::dynamic_fast_cast`
  * `lu::four_cc` and `lu::eight_cc`
  * `lu::endian_cast`
  * `lu::passkey`
  * `lu::singleton`
    * Rename `lu::singleton_ex` to `lu::singleton`; delete the old singleton template.

## Individual fixes to specific classes

* `cobb::vector3<T>` uses a `throw` statement when its `operator[]` is run in constexpr, to reject out-of-bounds accesses. This statement triggers compiler warnings when the operator is invoked; I should suppress those warnings. (Not doing that now because the header is used in enough places to have a fairly wide blast radius.)

## Specific files/definitions we can redesign/add

### `lu::passkey`

C++ has variadic templates, but there's no way to generate an arbitrary number of `friend` declarations, so it's not possible to define a passkey template class that grants access to an arbitrary number of classes. There's a proposal for variadic friends ([P2893R0 (PDF)](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2893r0.pdf)), but AFAIK it hasn't been accepted yet.

However, it seems that adding, like, seventeen `friend void;` declarations to a class doesn't break anything. We can thus define a passkey template that takes a large number of template types, with all of them defaulted to `void`.

```c++
#pragma once

namespace lu {
   template<
      typename A,
      typename B = void,
      typename C = void,
      typename D = void,
      typename E = void,
      typename F = void,
      typename G = void,
      typename H = void,
      typename I = void,
      typename J = void
   >
   class passkey {
      friend A;
      friend B;
      friend C;
      friend D;
      friend E;
      friend F;
      friend G;
      friend H;
      friend I;
      friend J;
      private:
         constexpr passkey() {}
   };
}
```

If your passkey needs to be used by more'n ten types, then you need to rethink a few things.
