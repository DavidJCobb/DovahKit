
# Loaded-form smart pointers

* Ergonomics suck: `stub->load().ptr_cast<SomeFormType>()` is ugly. We should have `stub->load<SomeFormType>()` instead where `SomeFormType` defaults to `dovah::loaded_forms::Form`, especially since `form_stub::load` literally just wraps `form_stub::_load` anyway.
* Don't allow implicit-casting from a smart pointer to a bare pointer. That's bitten me in a few places, where I reassigned a bare-pointer variable to `some_form->load().ptr_cast<SomeFormType>()` and thus ended up with that bare pointer dangling (i.e. assign it to temporary smart pointer's wrapped data; then temporary smart pointer destructs).
