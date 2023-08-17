
I needed (or just wanted, I guess) to do some basic Unicode checks on characters, i.e. "is this what Qt would consider printable?" but I wanted to do them in constexpr, as I was using these checks in code that I wanted to be able to run `static_assert`ions on for correctness.

I can't define `constexpr` metadata for every Unicode glyph, because ~~C++ doesn't have syntax for `constexpr` network access, which is probably a good thing~~ if I write JavaScript to parse [this listing](https://www.unicode.org/Public/UCD/latest/ucd/UnicodeData.txt) per [this spec](https://www.unicode.org/L2/L1999/UnicodeData.html) and generate even relatively basic structs of character metadata, the resulting array has over one million entries and is 191,992 lines long, and annihilates both IntelliSense and my computer's CPU.

The shortcut solution is to use JavaScript to parse the listing and generate C++ code that does range comparisons (i.e. `a >= 0x123 && a <= 0x456`) as often as possible and switch-cases otherwise.

Compilers limit how many instructions they'll execute during any given constant evaluation in order to prevent infinite loops from killing the entire IDE &mdash; a workaround for the halting problem. In order to reduce the number of instructions these functions execute, we split the Unicode listing into intervals (0x1000 characters per interval as of this writing); we have range checks and switch-cases inside of if-statements checking the intervals.

The result? Our `cobb::unicode::general_category_is_other` function, which returns the inverse of `QChar::isPrint`, is only about 1500 lines while handling all million-plus Unicode code points listed in the above sources.
