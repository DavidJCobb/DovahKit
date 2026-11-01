
# Form data fragment files

As of this writing, C++26 and its reflection functionality isn't available in MSVC, so I'm using X-macros as an alternative. Thus, `form_data` in DovahKit is a blend of preprocessor hackery and template metaprogramming, with fragment files used to make things easier.

In this context, fragment files are just `.inl` files that are `#include`d directly in the source code: sometimes they're included out in the open; other times they're included mid-struct-definition.

## General form

Here's the definition for Shouts with a few things ripped out, for a simple demonstration:

```c++
#include <array>
#include "../_base.h"

//
// Macros used as inputs to some of the fragments used below.
//
#define DOVAH_FORM_TYPE      shout // `dovah::form_type` enumeration member
#define DOVAH_FORM_CLASSNAME shout // identifier in `dovah::form_data` namespace

namespace dovah::form_data {
   template<form_data_params Params>
   struct shout : public _base<Params> {
      #include "dovah/form_data/fragments/pull_type_aliases.inl"

      //
      // nested type definitions here (structs, enums, etc.)
      //

      struct word_type {
         form_use spell;                // SNAM+0x04 -> SPEL
         form_use word_of_power;        // SNAM+0x00 -> WOOP
         float    recovery_time = 0.0F; // SNAM+0x08
         
         //
         // Fragment used to add poor-man's-reflection to this nested type. 
         // Preceding macros are used as inputs, and are (in this case) 
         // undef'd after usage.
         //
         #define CLASSNAME word_type
         #define TYPENAME  shout<FORM_DATA_PARAMS>::CLASSNAME
         #define PER_FIELD(X) \
            X(spell) \
            X(word_of_power) \
            X(recovery_time)
         #include "dovah/form_data/fragments/define_visitors.inl"
      };
      
      form_use                         equip_type;          // ETYP -> EQUP
      form_use                         menu_display_object; // MDOB -> STAT
      std::array<word_type, 3>         words;               // SNAM[3]
      
      //
      // Fragment used to add poor-man's-reflection to the outer `form_data` 
      // type. Preceding macros are used as inputs, and are (in this case) 
      // undef'd after usage.
      //
      #define CLASSNAME shout
      #define TYPENAME  CLASSNAME<FORM_DATA_PARAMS>
      #define PER_FIELD(X) \
         X(equip_type) \
         X(menu_display_object) \
         X(words)
      #include "dovah/form_data/fragments/define_visitors.inl"

      //
      // Another fragment used to enable construction of a managed-form-data 
      // instance by the "create blank instance" function table.
      //
      #include "dovah/form_data/fragments/enable_construction_via_table.inl"
   };
}

//
// Fragments used to declare some needed utilities.
//
#include "dovah/form_data/fragments/declare_tabled_functions.inl"
#include "dovah/form_data/fragments/define_form_type_constant.inl"

#undef DOVAH_FORM_CLASSNAME
#undef DOVAH_FORM_TYPE
```

## Files

### `declare_tabled_load_functions`

Use outside of any struct definition or namespace. Declares, but does not define, the functions used to load a form data type and to generate its use info:

* `dovah::form_data_function_tables::build_outbound_use_info::entry<...>::execute` (templated on a `form_type` constant)
* `dovah::form_data_function_tables::serialize_form_data::load` (overload for the managed form-data type)

Inputs are:

* **`DOVAH_FORM_TYPE`\:** The name of a member in the `dovah::form_type` enumeration.
* **`DOVAH_FORM_CLASSNAME`\:** The name of the form-data template type (e.g. `shout` for the example above).

### `define_form_type_constant`

Use outside of any struct definition or namespace. Specializes the `dovah::form_type_for_form_data_class` template, which maps form-data types to `dovah::form_type` constants.

Inputs are:

* **`DOVAH_FORM_TYPE`\:** The name of a member in the `dovah::form_type` enumeration.
* **`DOVAH_FORM_CLASSNAME`\:** The name of the form-data template type (e.g. `shout` for the example above).

### `define_visitors`

Use mid-struct-definition after the struct's fields. The inputs will be `#undef`'d. Defines type aliases and static member functions that allow iteration over an instance's non-static data members.

Inputs are:

* **`CLASSNAME`\:** The name of the class or struct being defined, e.g. `my_form_type` or `my_nested_struct`.
* **`TYPENAME`\:** The name of the class or struct being defined, qualified and including the nearest class that is templated on `form_data_params`, with the template parameter specified as `FORM_DATA_PARAMS`. Examples: `CLASSNAME<FORM_DATA_PARAMS>`, `my_form_type<FORM_DATA_PARAMS>::CLASSNAME`.
* **`PER_FIELD(X)`\:** An X-macro listing every field of the class or struct being defined.

This fragment defines the following members:

* **`static constexpr void visit_fields(CLASSNAME&, Callable&&)`\:** Executes the provided callable for each field of the passed-in instance, with a reference to the field passed to the callable. An overload is available for const and non-const references to the instance.

* **`static constexpr void visit_fields_in_tandem(CLASSNAME&, CLASSNAME&, Callable&&)`\:** Executes the provided callable for each field on the two passed-in instances; so, the callable runs once for each field, receiving a reference to that field on each instance. Overloads are available for all reference types (const/non-const; l-value/r-value).

* **Type alias `form_data_params_type`:** Whatever `form_data_params` the [enclosing] form-data type has been templated on. Used so that the params are retrievable for nested classes defined inside of a form-data class; our machinery for form-data-op functions requires this.

The `TYPENAME` input is used for cases where two instances that may be templated on different `form_data_params` should be usable together (e.g. `visit_fields_in_tandem`), so that we have a means to refer to the type of an instance templated on different params.

### `enable_construction_via_table`

Defines a constructor which takes a `lu::passkey` and a `form_stub&`. This is used to allow the `create_blank_instance` function table to create instances of the managed form-data type.

### `pull_type_aliases`

Type aliases defined in a class or struct are not inherited by its superclass, so C++ offers no convenient way to provide a collection of related type aliases to subclasses. This works around that, by pulling in common type aliases depending on the `form_data_params` that the form-data class is being instantiated with.

For example, in managed form data, `form_use` will be a `managed_form_use`, and in unmanaged data, it will be a bare `form_stub*`.
