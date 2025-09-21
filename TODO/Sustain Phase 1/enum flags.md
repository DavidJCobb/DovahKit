
# Enum flags

There are a number of flags-masks in Skyrim game data. It'd be nice if, instead of defining enum values as `1 << 3` or whatever, we could have the enum members' values be the bit indices, and have a helper class that maps them appropriately.

I have a `cobb::enum_flags` helper class, which is only used in a limited number of places, in part because it sucks.

## Limitations and flaws of `cobb::enum_flags`

* It's not possible to specify the number of defined enumeration values *and* the size of the flags-mask independently of one another. This bites us in a few places when handling furniture marker entry points (i.e. there are only 5 valid values, but we need 16-bit masks in NIFs, and in form data, some masks are 16-bit and some are 32-bit).

  * That is: we want to be able to specify the enum, the number of potentially meaningful bits[^potentially-meaningful], and optionally the size in bits (defaulted to just enough to hold the number of potentially meaningful bits). We want `cobb::enum_flags<Enumeration, PotentiallyMeaningfulBitcount, MinimumCapacityBitcount = PotentiallyMeaningfulBitcount>`.

  * Given two bitsets with the same underlying enum, it should be possible to assign them interchangeably if both have sizes-in-bits greater than or equal to the maximum meaningful bit. Currently, you can only "grow" an `enum_flags`; you can't assign a larger one to a smaller one; that's disallowed to avoid potentially truncating meaningful bits.

* Several member functions aren't `constexpr` when they should be.

* `number_set` should be called `count_set`, and should operate on more than one byte at a time. Similarly, `count` should be `size()`.

* There's no direct way to clear a range of values, unless those values are valid enumeration members specified at compile-time (i.e. `my_flags.reset<E::a, E::b>()`). This is a problem when dealing with flags-masks loaded from a file somewhere, where the data may not be "strictly" valid. The solution for discarding upper bits ends up being to construct a flags-mask temporary and then AND with it.

  * A member function like `clear_extra_bits()` would be nice to have, is what I'm saying.

* I actually hate the naming convention used by the STL for bitsets (set/reset for individual bits; clear for the whole set). I feel like `clear_one` and `clear_some` are clearer names than `reset`; and we should perhaps avoid the name `clear` for the whole set in favor of `clear_all`.

* Look at the code for `FurnitureMarkersModel` and friends in the UI. Anything that looks clunky, and anything that requires obtuse casts, is room for improvement.

[^potentially-meaningful]: If an enumeration has values 0, 1, 2, 3, 4, and 5, then the range of meaningful bits is \[0, 6), you need at least six bits to hold all meaningful flags, and so this template parameter should be 6. Consider an enumeration whose members' values are 1, 2, 3, and 5. In that scenario, bits 0 and 4 are not meaningful; the range of *potentially* meaningful bits is \[0, 6); you still need at least six bits to hold \[a canonical representation of\] the bitmask; and so 6 is still the template parameter. Thus, "number of potentially meaningful bits," defined as the index of the bit just beyond the highest meaningful bit.

## Desired features

* Add some kind of support for range-based for-loops. Probably best to have them produce a `std::pair<const value_type, bool>`, which could be used with destructuring assignment / structured bindings; this skips the need for loops to manually cast to the enum type -- at the cost of the minor overhead of building a struct, if the compiler doesn't recognize that the pair is just a loop index, and optimize accordingly. (Maybe `std::pair<const value_type&, bool>`, with the reference being to the loop index, would fare better? Maybe in that scenario the compiler would recognize that the pair struct is superfluous and just serving as an interface, and optimize it out? We could check it on Godbolt.)