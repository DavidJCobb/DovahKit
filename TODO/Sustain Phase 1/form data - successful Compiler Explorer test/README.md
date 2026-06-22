
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


## Additional uses for "visit" functions

For a while, I've been considering whether and how DovahKit should handle mistyped form uses, e.g. a form specifying a `STAT` when an `NPC_` was expected. Currently, DovahKit's backend leaves these uses intact when loading form data, merely warning upon encountering them. The frontend's various UI widgets generally end up auto-correcting these uses as you edit forms while using the program, but in general, code can't make assumptions about the type of form that a form use points to, even when the types it *should* point to are known.

I've considered having DovahKit silently correct these form uses to `NONE`, or making that an option that form-data classes are templated on. It occurs to me now that I could write a function that uses `visit_fields` to perform that correction, modifying form-data (managed or unmanaged) in place.


## Critical missing pieces

### Idle animation jank and non-severable form uses

There is one known case where it isn't always possible to sever a form use: an Idle setting an Action as its parent. Ordinarily, this would make the Idle the "root" for that Action. However, due to mistakes that Bethesda made when implementing code to load the Idle Animations tree, it's possible for multiple Idles to fight over the same Action, with the winning Idle being whichever one has the latest-loading record that lays claim to the Action; it's also possible for a single Idle to be a candidate for multiple Actions. Thus for every Idle we have to track two lists of "action root candidacies:" one for Idle -> Action uses that come from dependencies, and one for Idle -> Action uses that come from the active file.

We can't sever an Idle's action-root candidacy (i.e. by severing the Idle -> Action use) if that candidacy comes from a dependency file (i.e. a file other than the active file). The current code for the `IdleAnimation` form works around this by violating the contract for the "sever uses of form" function, with the following knowledge:

* Refusing to sever the use of a non-active-file form, in violation of the function's contract, will not lead to a dangling pointer.
  * The "sever" function is only invoked when "deleting" the used form.
  * If the used form is not defined in the active file, then "deleting" it just means setting a record flag on it; the used form's `form_stub` continues to exist.
* By definition, the list of action root candidacies that were loaded from non-active files cannot contain candidacies for an active-file Action. This is the only list for which we may need to refuse, and it can only contain forms that (per the above bullet points) we are allowed to refuse.

Still, this is janky as hell.

For this reason, instead of having a general-purpose `sever_uses_of_form(form_stub&)` function, we should only offer a specific `sever_uses_of_form_pending_deletion(form_stub&, form_deletion_type)` function, given some `enum class form_deletion_type` that defines members `flag_record_as_deleted` and `wholly_delete_form_and_stub`. We should also define a `non_severable_form_use` type for this specific case, which is skipped if the used form is only being flagged, and which fails an assertion if the used form is being wholly deleted.

(I'm actually considering having the backend manage idle trees outside of the forms themselves, e.g. with some `active_idle_tree` struct that would be a sibling to `form_stub_collection` within the `active_load_order`. We could defer all handling of this problem to that struct. However, if the Idle's managed form data doesn't contain the candidacies and the computed hierarchy positions (parent and previous-sibling form), then its Use Info won't list those forms. At the same time, we want the *un*managed form data to only contain the computed positions, on retrieval, and for you to update the requested positions by overwriting that and committing the data. Hm...)


### Support for transaction-style structs

Conditions (i.e. `TESCondition`) in DovahKit use a transaction-style model, wherein you can't modify the managed condition data directly; you can only "commit" an unmanaged condition in full, with us asserting that its data is well-formed. This is necessary because condition parameters are basically tagged unions where the function ID and some of the flags collectively function as the parameters' tag, so if we let you store invalid parameters and we try to serialize those to disk, they'll later be loaded improperly and our use info will be hosed.

This is incompatible with the above design, which assumes that you can convert between (un)managed data just by visiting a list of `public` fields. (The design also assumes that it's possible *at all* to visit all the fields in the given object.) So we face two problems: how do we convert between (un)managed data, i.e. how do we implement the transaction "commit" code; and how do we visit fields in managed data, e.g. for clearing form uses?


#### Conversion

We need to add a special-case system: it needs to be possible to:

* Manually define separate types for the managed versus unmanaged kinds of a given data structure
* Manually define conversions between those types
* Have some means of linking these types together *and* some means of detecting *either structure* from within the metaprogramming machinery (visitation, etc.) described above

Possibly we can solve this with a specializable type trait:

```c++
template<typename T>
struct managed_type_to_unmanaged_type;
template<typename T>
struct unmanaged_type_to_managed_type;

template<typename T>
using managed_type_to_unmanaged_type_t = typename managed_type_to_unmanaged_type<T>::type;
template<typename T>
using unmanaged_type_to_managed_type_t = typename unmanaged_type_to_managed_type<T>::type;

// -----------------------------------------

struct managed_condition;
struct unmanaged_condition;

template<>
struct managed_type_to_unmanaged_type<managed_condition> {
   using type = unmanaged_condition;
};
template<>
struct unmanaged_type_to_managed_type<unmanaged_condition> {
   using type = managed_condition;
};
```

This fails if we ever add additional fields to `form_data_params`, though, because it can only convert between specific specializations of the form data in question (or its containing template). Like, it allows conversion between "managed" and "unmanaged," but if we introduce new "struct colors" (in the same sense as "function colors"), this will fail to account for those colors, or for different combinations of those colors and the existing colors (e.g. "unmanaged+foo type" -> "managed+bar type").

An alternative approach, which avoids that failure, would be to modify the `inl` file that adds metaprogramming stuff to our various form data structs. We could define something like:

```c++
#define FORM_DATA_PARAMS OtherParams
template<form_data_params OtherParams>
using respecialized_type_with_params = TYPENAME<FORM_DATA_PARAMS>;
#undef FORM_DATA_PARAMS
```

This would then allow the more typical structs to expose somthing like `my_struct::respecialized_type_with_params<unmanaged_form_params>`, and then `managed_condition` and `unmanaged_condition` could manually offer a similar type alias. This gives us a means to ask for the right type, and then we can implement conversion functions to and from managed transaction-style types based on that templated type alias, e.g. member functions like these on the "managed condition" type:

```c++
template<form_data_params Params>
   requires (Params::management_mode == use_info_management_mode::unmanaged)
struct unmanaged_condition;

template<form_data_params Params>
   requires (Params::management_mode == use_info_management_mode::managed)
class managed_condition;

namespace impl {
   // IIRC can't use `conditional_t` to select the above templates, as their `requires` 
   // clauses would be failed in some of the `conditional_t` "branches." a type alias 
   // in a specialized template will work instead.
   template<form_data_params>
   struct condition_t;
   
   template<form_data_params P>
      requires (Params::management_mode == use_info_management_mode::unmanaged)
   struct condition_t<P> {
      using type = unmanaged_condition<P>;
   };
   
   template<form_data_params P>
      requires (Params::management_mode == use_info_management_mode::managed)
   struct condition_t<P> {
      using type = managed_condition<P>;
   };
}

template<form_data_params Params>
   requires (Params::management_mode == use_info_management_mode::managed)
class managed_condition {
   public:
      
      // ... put whatever boilerplate our visitor metaprogramming needs, HERE ...
      
      template<form_data_params OtherParams>
      using respecialized_type_with_params = impl::condition_t<OtherParams>::type;
   
      template<form_data_params DstParams>
         requires (DstParams::management_mode == use_info_management_mode::unmanaged)
      respecialized_type_with_params<DstParams> convert_to_unmanaged() const;
         
      template<form_data_params SrcParams>
      void overwrite(const respecialized_type_with_params<SrcParams>&);
};
// definitions of those member functions can go in a separate `inl` file, for brevity
```


#### Visitation

This... I'm actually not 100% sure about. ~~We may literally have to just special-case these kinds of structs in the individual visitor functions. There aren't many of these structs at all (conditions are the only one I can think of), so for now that may be fine?~~ No, that wouldn't scale. Hm...

* The key thing is that when we're visiting managed form data, a managed transaction-style struct can only be visited via a const reference (because again, the fields can't be individually manipulated), and doing so would require invoking getters for each field (because to prevent individual manipulation of the fields, they must be made non-`public`). For read access only, we could have an optional X-macro of getter names to invoke...

* ...but some visit functions, e.g. "clear uses," require write access. I think the only way to make this work would be to make it so that when these functions are invoked on a managed transaction-style struct, they convert it to unmanaged, make their changes to that, and then commit that unmanaged struct back overtop the original managed struct.


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