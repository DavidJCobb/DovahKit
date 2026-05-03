
As of 5/3/2026, these files compile properly when you create an IDE View in Compiler Explorer, add each file, include them all in the project, and compile with `x64 msvc v19.latest` given `/std:c++latest /O2`. Enable CMake, pass the args `CMAKE_BUILD_TYPE=RelWithDebInfo -DCMAKE_BUILD_TYPE=Release`, and set the expected output to `multifile.exe`.

----

This is a practical test of the way I want to template form data, using X-macros and metaprogramming to make it possible to generate "managed" and "unmanaged" data for a single form type given a single template. To wit:

* `dovah::form_data_ops::assign` can convert between managed and unmanaged data.
* `dovah::form_data_ops::clear_managed_data` can clear all form-uses out of a managed form-data object. (Making it usable on unmanaged form-data, able to clear bare `form_stub*`s, should be trivial.)
* More generally:
  * For any given form-data type or nested type, `T::visit_fields(instance, lambda)` will invoke `lambda` on each field. The `instance` may be any reference (l/r) and any constness.
  * For any given form-data type or nested type, `T::visit_fields_in_tandem(a, b, lambda)` will invoke lambda on each pair of matching fields in `a` and `b`. The instances `a` and `b` may be any reference (l/r), any constness, and templated on any `form_data_params`, and they may differ from each other in any of those respects.
  * On these foundations, we can build things like `clone_from(T& lhs, T& rhs)`, or `on_other_form_deleted(T& subject, form_stub& to_be_deleted_form, bool just_being_flagged)` for severing references.

Because this is just a proof of concept, it's very limited; for example, functions like `dovah::form_data_ops::assign` don't handle any containers[^containers] besides `std::array`. It also doesn't handle heap-allocated sub-objects (e.g. quest aliases); it can't follow pointers. (I think we'd want to use `std::unique_ptr` more consistently for heap-allocated sub-objects.)

[^containers]: Worth noting that for resizable containers e.g. `std::vector`, we'll want customized subclasses or alternative types. For managed form data, overwriting or removing elements from the container should properly invoke managed operations (e.g. `managed_form_use::set(form, nullptr)`), and therefore these mutations will need to take the enclosing form as an extra parameter.

## Overview

The definition of the Shout form type looks like this:

```c++
namespace dovah::form_data {
   template<form_data_params Params>
   struct shout : public _base<Params> {
      #include "define_form_data_type_aliases.inl"

      struct word_type : public type_aliases<Params> {
         form_use spell;
         form_use word_of_power;
         
         #define CLASSNAME word_type
         #define TYPENAME  shout<FORM_DATA_PARAMS>::CLASSNAME
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power)
         #include "define_form_data_visitors.inl" // undef's the above three macros
      };
      
      common_localized_string_use      name;                // FULL
      description_localized_string_use description;         // DESC
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]

      #define CLASSNAME shout
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(name) \
         X(description) \
         X(equip_type) \
         X(menu_display_object) \
         X(words)
      #include "define_form_data_visitors.inl" // undef's the above three macros
   };
}
```

`Params` is a struct type with `static` members indicating whether the form data is managed. (A struct is used to allow for future extensibility.)

The `shout` class includes a fragment to pull in useful type aliases that are contingent on whether the form is managed; for example, `form_use` resolves either to a bidirectionally tracked pointer, or to a bare pointer. (Defining these type aliases on the base class doesn't work; they don't automatically get inherited.) The base class, `_base`, is a type alias resolving to either `base_managed_form_data` or `base_unmanaged_form_data`; these structs define stub-level content (i.e. a `form_stub&` reference for the former; for the latter, a struct with record flags, an editor ID, and so on), and they're also useful in and of themselves (e.g. `managed_form_use::set(base_managed_form_data&, form_stub*)` would be the counterpart to `form_reference_t::set(loaded_forms::Form&, form_stub*)` in DovahKit's alpha).

Both `shout` and `shout::word_type` use X-macros, configuration macros, and an included fragment to define critical static members: `form_data_params_type`, to make `Params` accessible to the outside world[^form_data_params_type]; `visit_fields`, allowing a lambda to be invoked on every field on an instance; and `visit_fields_in_tandem`, allowing a lambda to be invoked on every matching field on a pair of instances (which may differ in constness, referenceness, and `Params`).

[^form_data_params_type]: The `form_data_params_type` type alias is needed so that templates can check whether a given type `T` is directly *or indirectly* templated on form data params. (That capability, in turn, is needed so that functions wishing to visit members of an outer type can tell when they encounter a field that is, itself, contingent on params, i.e. a field over which they should recurse.) It's not sufficient to just check whether `T<Params>` is valid because, using the example above, that wouldn't work for `shout<Params>::word_type`: the `word_type` is not a template in itself, but rather is a non-template member of a template; the only way to tell that `word_type` is contingent on form-data-params is to expose the params as `word_type::form_data_params_type`.


## Potential improvements

In addition to defining "visit" functions, it may be useful to offer a `visit_at_compile_time` function, which would be invoked as `lambda.template operator()<typename FieldType>(std::string_view{field_name})`. This would allow us to run compile-time queries on the types of an object's fields, without needing to default-construct an instance in order to call `visit_fields` on it. Potential uses of these queries include:

* Verifying that a form-data type doesn't include any inappropriate fields (e.g. verifying that managed form data has no bare `form_stub*` pointers).
  * We hae a less robust implementation of this in `sanity.cpp`, which relies on invoking `visit_fields` on default-constructed objects.
* Checking whether a form-data type (or any sub-object thereof) has any fields that would even need to be managed. (Functions like `dovah::form_data_ops::clear_managed_data` could use this to skip sub-objects that have no `form_use`s or localized strings in them. Without the ability to query this in advance, these functions would end up walking all fields and nested sub-objects, even when doing so is a no-op.)
  * These checks could also be used to allow fast-path `lhs = rhs;` for `dovah::form_data_ops::assign`.

That function should be trivial to define; off the top of my head, it'd be something like:

```c++
template<typename Lambda>
static consteval void visit_at_compile_time(Lambda&& f) {
   #define X(name, ...) std::forward<Lambda>(f).template operator()<decltype(name)>(std::string_view(#name));
   PER_FIELD(X)
   #undef X
}
```

And then, based off that and `visit_fields`, we could even go insano style and do something like this:

```c++
/*
   Code to print the contents of an object to the console:
*/

namespace impl {
   template<typename field_type>
   constexpr void print_scalar_value(const T& field) {
      if constexpr (std::is_floating_point_v<field_type>) {
         std::printf("%f", field);
      } else if constexpr (std::is_integral_v<field_type> && std::is_arithmetic_v<field_type>) {
         if constexpr (std::is_signed_v<field_type>) {
            std::printf("%d", field);
         } else {
            std::printf("%u", field);
         }
      } else if (std::is_enum_v<field_type>) {
         std::printf("enum, underlying value %d", (int)field);
      } else if (std::is_same_v<field_type, bool>) {
         if (field)
            std::print("true");
         else
            std::print("false");
      } else if (
         std::is_base_of_v<std::string, field_type>
      || std::is_base_of_v<std::string_view, field_type>
      ) {
         std::print(field);
      } else {
         std::printf("???");
      }
   }
}

template<uses_form_data_params T>
constexpr void print(const T& instance, size_t indent = 0) {
   constexpr const auto all_field_names = []() {
      constexpr const size_t count = []() {
         size_t v = 0;
         T::visit_at_compile_time([&v]<typename Field>(std::string_view) {
            ++v;
         });
         return v;
      }();
      std::array<std::string_view, count> names = {};
      size_t i = 0;
      T::visit_at_compile_time([&names, &i]<typename Field>(std::string_view name) {
         names[i++] = name;
      });
      return names;
   }();
   
   size_t i = 0;
   T::visit_fields(instance, [indent, &i](const auto& field) {
      using field_type = std::decay_t<decltype(field)>;
      
      std::printf("% *s - %s == ", (indent * 3), "", all_field_names[i]);
      
      if constexpr (uses_form_data_params<field_type>) {
         std::printf("{\n");
         print(field, indent + 1);
         std::printf("% *s}", (indent * 3), "");
      } else if constexpr (std::is_pointer_v<field_type> && std::is_base_of_v<form_stub, std::remove_pointer_t<field_type>>) {
         // ... print a string of the form "[TYPE:00123456]EditorID" ...
      } else if constexpr (std::is_base_of_v<managed_form_use, field_type>) {
         // ... print a string of the form "[TYPE:00123456]EditorID" ...
      } else if constexpr (lu::std_array<field_type>) {
      
         //
         // this isn't fully thought-out... i need to figure out how to factor 
         // this out so we can handle scalars, containers, and sub-objects all 
         // in a graceful way and with minimal code duplication
         //
         // probably `print` should be multiple different functions: each top-
         // level function should be concerned with printing a different entity 
         // type.
         //
      
         std::printf("{\n");
         for(size_t i = 0; i < field.size(); ++i) {
            const auto& item = field[i];
            std::printf("% *s[%u] == ", ((indent + 1) * 3), "", i);
            if constexpr (uses_form_data_params<field_type>) {
               print(item, indent + 1);
            } else if constexpr (is_container<field_type>) { // TODO
               // TODO
            } else {
               print_scalar_value(item);
            }
            std::printf(",\n");
         }
         std::printf("% *s}", (indent * 3), "");
      } else {
         print_scalar_value(field);
      }
      std::printf(",\n");
   });
}
```