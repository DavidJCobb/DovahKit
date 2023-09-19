
* Type-safe flags-masks all over the backend. It's trivial to do this with templates now, so the "non-strict enum inside a struct" trick isn't needed anymore.
  * That said, the "non-strict enum inside a struct" trick allows "subclassing enums" when one wrapper struct subclasses another. Never tried doing that with template-based flags masks before...
    * Constexpr validation of flags, in the context of subclassable/extensible flags masks, would require explicit object parameter support ("deducing `this`"), which is busted in IntelliSense as of this writing (9/18/2023; definitions work; calls show false-positive errors).
* Better access specifiers for `form_stub`'s members. Lots of stuff is public when it should arguably be private with a `constexpr const auto&` getter and maybe some passkeyed non-const accessors.
* Revise `loaded_form_ptr` to be much more ergonomic. Having to get an untyped smart pointer via `form_stub::load` and manually call `ptr_cast` on it is annoying.
* Rename `dovah::loaded_forms` to just `dovah::forms`. We'd still want to call these things "loaded form classes," but naming the namespace that feels redundant; that you have an instance of one of these classes on hand implies that you've loaded it.
* Move `dovah::loaded_forms::components` to `dovah::form_components`.
* Change how form components are handled:
  * Have a loaded form class define an anonymous `components` member struct.
  * Put the relevant loaded form component members inside of that struct.
  * Have a dispatch table that can access a component from any form that actually contains it, accounting for `std::optional<T>` components like destruction data.
    * We already have accessors defined as member functions on `Form`. It'd be nice to have a single accessor templated on the desired component type; however, it'd have to be non-member rather than a member of the base class, e.g. `dovah::component_of<T>(form)` or `dovah::form_components::get<T>(form)`, as whatever form pointer you use needs to be a derived pointer, not a base class pointer.
* Change how forms handle embedded references to other forms:
  * Currently, to clone a form that contains references to other forms, we instantiate an empty destination form (`dst`) and then copy from the source form (`this`) in the latter's `_clone_impl` virtual member function. Copying those references, then, looks like this: `dst->some_ref.set(*dst, this->some_ref)`. It'd be nice if we instead had a dedicated "clone form reference" function that would take as arguments the source form, the destination form, and a pointer-to-member.
    * Bonus points if the pointer-to-member can be to anything: if it's a `std::vector<form_reference_t>`, then we loop over it; if it's a custom struct, it can define its own "clone" function that we'll then invoke via templating.
    * And ditto for helper functions like `copy_form_reference_list`.
    * Ditto for severing outbound references to a target form, and for clearing data, too.
  * If possible, we should set `form_reference_t::operator=(const form_reference_t&) = delete`. This would significantly reduce the room for error when writing forms' boilerplate.
* Improvements to form extra-data:
  * Replace `template<class E> E* extra_data_list::lookup(extra_data_type et)` with a getter that doesn't take any arguments; use a `constexpr` mapping of extra-data classes to typecodes to know what typecode to look for.