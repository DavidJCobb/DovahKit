
# Sound descriptor

Defines a sound, as a collection of sound files that can be selected at random.

## Class names (CNAM)

The `BGSSoundDescriptorForm` is capable of holding multiple sound descriptor types, but in practice, only one actually exists: `BGSStandardSoundDef`. Internally, there is a `BSTObjectDictionary`, a hashmap, which maps CRC-32 hashes[^crc] of typenames to factories. Note my wording: it does not map typenames to factories. Rather, Bethesda hashes the typename before storing or doing lookups, and then it gets hashed *again* within the internals of `BSTObjectDictionary`.

`SNDR/CNAM` is a CRC-32 hash of the classname being used (so, a hash of `BGSStandardSoundDef`). Since `BGSStandardSoundDef` is the only sound type that exists, the subrecord should always be `0x1EEF540A`. Under the hood, when the game does lookups, it will hash this *again* to `0x27E1B448` and search the internal `BSTObjectDictionary` storage for that hash, to find the factory used to instantiate ` BGSStandardSoundDef`.

[^crc]: If you wish to test Bethesda's hashes, [this online hasher](https://www.sunshine2k.de/coding/javascript/crc/crc_js.html) works as of this writing. Select CRC-32 with custom parameters; check "Input reflected" and "Result reflected," set the polynomial to 0x04C11DB7, and set the initial and final-XOR values to 0.

When the game sees `CNAM`, it looks for a factory; if it finds one, it instantiates the sound data, opens the next subrecord ,and calls into the sound data's loader function. `BGSStandardSoundDef`'s loader consumes all remaining subrecords. Only subrecords `EDID`, `VMAD`, `OBND`, and `CNAM` are processed by `BGSSoundDescriptorForm` itself; all other subrecords belong to `BGSStandardSoundDef` and if they appear before `CNAM`, they will be ignored.

