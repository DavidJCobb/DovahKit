
# Class names identified by hashes

Some form types store the bulk of their data using an inner object of variable type (hereafter: "typed data"), with the type in question identified by CRC-32 hashes. These include Sound Descriptors and Music Tracks:

* Music track forms are `BGSMusicTrackFormWrapper`. They are capable of storing multiple kinds of track data, all of which subclass `BSIMusicTrack`: `BGSMusicSingleTrack`, `BGSMusicSilenceTrack`, and `BGSMusicPaletteTrack`.

* Sound descriptor forms are `BGSSoundDescriptorForm`. They are technically capable of storing multiple kinds of sound data (via base class `BSISoundDescriptor`), but only one such class exists: `BGSStandardSoundDef`.

The way these work is that internally, for each form type in question, there is a hashmap (most likely `BSTObjectDictionary`) which maps CRC-32 hashes[^crc] of classnames to [factories](https://en.wikipedia.org/wiki/Factory_method_pattern). Note my wording: it does not map classnames to factories; rather, Bethesda hashes the classnames before storing or doing lookups, and then they get hashed *again* within the internals of dictionary.

[^crc]: If you wish to test Bethesda's hashes, [this online hasher](https://www.sunshine2k.de/coding/javascript/crc/crc_js.html) works as of this writing. Select CRC-32 with custom parameters; check "Input reflected" and "Result reflected," set the polynomial to 0x04C11DB7, and set the initial and final-XOR values to 0.

`MUST/CNAM` and `SNDR/CNAM` are CRC-32 hashes of the classname being used. Under the hood, when the game does lookups, it will run CRC-32 on the hash a second time, and search the internal dictionary for that hash, to find the factory used to instantiate the typed data. If it finds a factory, it will use that factory to create a new instance of typed data, and then open the next subrecord and call into the typed data's loader function. Typically, that function will consume all remaining subrecords.

So for example, when dealing with a Music Track form, only the EDID, VMAD, OBND, and CNAM subrecords are seen by the form's own loader. If CNAM is equal to `0x23F678C3` (the hash of the string "BGSMusicPaletteTrack"), then the loader will hash that hash to look up the factory for `BGSMusicPaletteTrack`, use that factory to create an instance of that class, and feed subrecords to that instance's "load" function until the function exits.

Each collection of typed data has its own dictionary, so for example, the hash `0x23F678C3` ("BGSMusicPaletteTrack") would not be recognized if placed in `SNDR/CNAM`.

The code (continuing with music tracks as an example) looks very roughly like this:

```c++
bool BGSMusicTrackFormWrapper::Load(TESFile* file) {
   //
   // ... boilerplate ...
   //
   uint32_t signature = file->subrecord_signature;
   while (signature) {
      switch (signature) {
         //
         // ... other subrecords ...
         //
         case 'CNAM':
            {
               uint32_t hash;
               file->Read(/*uint32_t&*/hash, sizeof(hash));
               if (file->is_big_endian) {
                  hash = std::byteswap(hash);
               }
               this->typed_data = MakeTypedDataByHash(hash); // BSIMusicTrack*
               if (this->typed_data)
                  this->typed_data->Load(file); // virtual 0x0C
            }
            break;
      }
      if (!file->next_subrecord())
         break;
      signature = file->subrecord_signature;
   }
   return true;
}

/*static*/ BSIMusicTrack* BGSMusicTrackFormWrapper::MakeTypedDataByHash(uint32_t hash) {
   using factory_type    = IBSTCreator<BSIMusicTrack>;        // known
   using dictionary_type = BSTObjectDictionary<factory_type>; // assumed

   dictionary_type&    dictionary = /*static storage duration*/ type_dictionary;
   const factory_type* factory    = dictionary.lookup(hash); // hashes the hash again
   if (!factory)
      return nullptr;
   return factory->Create(); // virtual 0x01
}
```

Bethesda uses polymorphic classes for typed data, but we can just use `std::variant`s. We don't need to plan ahead for a potentially unbounded number of classes for any given typed data (i.e. we already know how many sound descriptor classes there are, how many music track classes there are, et cetera) and (because we aren't making an actual game engine that needs to actually use this data very often) we don't need rapid access to these classes' functionality (i.e. `std::holds_alternative` and `std::get` are fine for us; we don't need virtual calls).