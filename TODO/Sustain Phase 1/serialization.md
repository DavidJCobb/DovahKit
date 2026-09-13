
# Serialization notes

## Mapping record IDs to form IDs

Internally, this process is referred to as "fixing up form IDs;" in the rewrite it should be referred to as "resolving" "record IDs" to "form IDs."

Currently, if we see a record ID that is out-of-bounds within its containing file, we return an error. For form uses, this means that the use fails to resolve. This is mostly correct but for one thing: the game itself instead acts as though the record ID used the load order prefix of the containing file.

For example, consider a load order with Skyrim.esm, Update.esm, IrrelevantFile.esp, OtherUnrelatedFile.esp, and MyCoolMod.esp. Suppose that MyCoolMod lists Skyrim and Update as its masters, such that record IDs prefixed with 02 refer to forms defined within MyCoolMod itself; thus the record IDs would resolve to load order prefix 04. If there exists a use of a form whose specified ID is 03xxxxxx, then we error on that and resolve it to 00000000, whereas the game would pretend that the record ID was actually 02xxxxxx and thus resolve it to 04xxxxxx.

This is mostly only relevant for cases where the game inadvertently resolves form IDs twice (i.e. DLBR/QNAM), as this is what prevents DLBRs in all new content files from being *intrinsically* broken (i.e. you can add DLBRs to your own quests, but not to quests defined in any master, unless that master has the same position in your master list as in the final load order -- generally only true for Skyrim.esm and the DLCs assuming a well-formed master list). If we want to load a malformed file faithfully to how the game would load it, however, then we should replicate the game's behavior and add a special warning for this specific case. The warning would have to be emitted from within the subrecord-reader object itself; I don't want each individual form loader to have to manually check for this sort of thing.

For now, I special-case the logic for detecting when DLBR/QNAM will break; so, resolving record IDs to form IDs is is strict in DovahKit (i.e. out-of-bounds record IDs resolve to null rather than to forms in the containing file), but the code that detects when DLBR/QNAM will break will manually apply the "same file" logic on its own to avoid emitting spurious warnings on basically every DLBR outside of Skyrim.esm.

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

## Memory management during writes
Currently, every individual value written reallocates the destination buffer for subrecords. We could optimize this if we used similar behavior to `std::vector` and friends and eagerly pre-allocated. That is: if we want to write e.g. a four-byte value, but the buffer doesn't have capacity, then don't just allocate to `current_size + 4`; allocate to `current_size + 64`. Like, expand the buffer in 64-byte chunks whenever we're not writing something larger than that many bytes.

This may potentially even be more efficient than having tons of structs manually call `reserve_more` (plus, with the "multi-write" change proposed above, we could automate `reserve_more` calls rather than the callers manually needing to calculate and run them).