
The loader for `ExtraRoomRefData` is very, very janky.

* The loader knows what data to expect based on a set of presence flags in the initial `XRMR` subrecord. Each flag signals the presence of two upcoming subrecords: in order, `LNAM` and `INAM`: the lighting template and the imagespace. Crucially, the loader blindly opens the next subrecord without double-checking its signature. In other words, if it *expects* `LNAM`, then it treats the next subrecord *as* `LNAM` regardless of its actual signature.

* Similarly, `XRMR` contains the number of linked rooms to expect. After the game has opened `LNAM` and `INAM` as appropriate, it then expects `XLRM` subrecords in the specified count, and will blindly open that many subrecords.
  
  There's a wrinkle here, though: the game *does* check for superfluous `XRMR` subrecords, and will skip them. So really, if the initial `XRMR` specifies *n* subrecords, then the game will open the next *n* non-`XRMR` subrecords, skipping any `XRMR` subrecords interleaved between them.