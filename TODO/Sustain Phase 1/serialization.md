
# Serialization notes

## Multi-read and multi-write
The "read" and "write" functions used to read TES files only stream one field at a time. In other programs I've written since I wrote the TES file code, I've had multi-stream functions &mdash; variadic template functions which read/write multiple values in sequence. It'd be nice to bring that enhancement to DovahKit.

## String subrecords
The subrecord-reader interface has a `read` overload for `std::string` which pulls the entire content of the subrecord into the string. This is, frankly, stupid, because there are many ways a string could be stored within a subrecord:

* Null-terminated string
* Length-prefixed string, possibly with a redundant null terminator
* The entire subrecord is one string

One could quite reasonably expect a `read` call on a `std::string` to read a null-terminated string; having it read the full subrecord is a footgun. Basically, we should have separate member functions for all of the above cases; possibly we shouldn't have any `read` function that's compatible with a `std::string`.

## Support for structs that have "value semantics"
Some structs should be treated as "values" rather than as "objects." However, IIRC it's not possible to define the `read` behavior for the subrecord reader. You can define a `load` member function on a struct, and call that; but you can't just pass arbitrary structs to the subrecord reader's `read` function.

The main use case here is package data UIDs. I could define them as `using package_data_uid = uint8_t`, but then they become interchangeable with any other unsigned byte, and we can't define convenient members (e.g. `package_data_uid::none` as constant `0xFF`). It's not hard to make a struct that can be templated on any integral type, with explicit casts and reimplemented integer operations, such that integers that we want to be unique (i.e. not interchangeable with other integral types except via a manual cast) can be subclasses of a template instantiation (and we can then give them static member constants as desired), but AFAIK/IIRC DovahKit's current serialization code offers no way to seamlessly adjust serialization behavior for arbitrary structs (i.e. we'd have to give the type its own members that take subrecord readers/writers by reference, and call `my_id.read(subrecord)` instead of being able to do `subrecord.read(my_id)` as with any other integral or scoped enum).

Color dwords are another common struct where it'd be nice to just shove them into the `read` function.

As for how to expose this kind of customization, without making said types directly dependent on the subrecord reader types? I could look into how `std::format` exposes customization and see if there are techniques I can borrow.