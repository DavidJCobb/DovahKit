
# Compiled Papyrus script files

## Notes

* When the game sees a scriptname in VMAD, it will look for the PEX file whose filename matches that scriptname, and then search the object definitions in that PEX file for one with the desired scriptname, ignoring all others it finds. (It also doesn't stop at the first match, so it's the *last* match that the game uses.) This means that while it's theoretically possible to embed multiple scripts in a single PEX, only one whose name matches the PEX filename will actually be successfully found and used by the game.

  Actually, it's wackier than that, because...

  * The size field for PEX objects is encoded incorrectly by Bethesda's tools (or read incorrectly by Bethesda's game; either or, really.) All Bethesda tools encode the size so that it includes itself (i.e. it's a `uint32_t`, so the value is 4 larger than it should be), but the game doesn't take that into account.
  
    When the game wants to load a given scriptname, it'll open the PEX file whose filename matches that scriptname, and then loop over all embedded objects. When it sees an object with a non-matching scriptname, it'll attempt to skip that object without reading it. It does so by reading the size as *n* and then skipping forward by *n* bytes... but since Bethesda's tools generate PEX files where *n* includes *the size of* n *itself*, the effect is that they (having already read *n*, and being located after its end) skip four bytes past where they want to go, and end up reading garbage for all subsequent objects. The game needs to subtract 4 for this whole thing to work, and it doesn't.

    In practice, the file format supports multiple objects in a single PEX, but this only works reliably for the first object in a PEX (unless the size fields are fixed via hex editor to not count themselves), or when a PEX is self-overriding (i.e. the first *u* objects are all the same script, for any given value of *u*, such that none of them are skipped and the last of them overrides all the others). And again, only objects that match the filename can ever work anyway; all others are skipped by design, and then never looked for in the right place.

    * In turn, this also means *we* can't trust the size field when skipping, in part because it could've been hex-edited and in part because a hex-edit could be meant to correct the in-game behavior, or to just cause chaos. Basically the field is worthless to us.

* In turn, the only use for "multi-PEXs" &mdash; one PEX file with multiple object definitions &mdash; is as an obfuscation technique, since as of this writing Champollion only decompiles the first object in a PEX (though it'd be trivial for them to fix that). You could confuse or misdirect Champollion with either of the following techniques:

  * Put two objects in the PEX file. The former should have a scriptname that doesn't match the PEX filename, and should have a size field that has been fixed via a hex editor (or generated via a third-party tool that matches the game's loader): this script is a dummy script that the game will skip. The latter should be the real script, with a name matching the PEX filename.

  * Put *n* objects in the PEX file. All of them should have a scriptname matching the PEX filename. The last of them is the real script; all prior to that are fakes. In this case, the game never skips any of them (because they all have a name matching what the game is looking for), so the bad size field is irrelevant.

  In turn, if someone wanted to edit Champollion to resist these obfuscation techniques, the approach I'd recommend is to decompile all objects in a PEX and just write them to different files. The file naming convention I'd personally go with when decompiling a multi-PEX is: `"%s.%s.psc"`, given the PEX filename and embedded scriptname; or, when multiple objects in the PEX have the same scriptname, `"%s.%s[%d].psc"`, given the PEX filename, embedded scriptname, and number of times that scriptname has been seen so far.

* There is no "none" sentinel value for string indices; instead, encode an empty string into the string table and use that.
* A PEX object with no states is invalid; there must at least be an empty state (state with a zero-length name). Unsure if a PEX object can have only non-empty states; I assume so.
* A message exists for if a PEX object has two states with the same name; unsure if this is a warning or an error.
* The game emits warnings if a PEX object contains a variable with a non-constant initial value, i.e. if the variable's type is `raw_type::object`.
* Unused string table entries in a PEX will not be added to the game's in-memory string table.
  
  The PEX loader first grabs the string table's contents as a vector of `const char*`, and creates a parallel vector of `BSFixedString` (tabled strings) all initialized to an empty string; we'll call these lists *C* and *B*. Every time the PEX loader reads a string index *i* from the PEX, it checks if *B[i]* is the empty string; if so, then *B[i]* is initialized to *C[i]*.