
# File and form versions

All records have a version number in their headers. Skyrim and the Creation Kit are capable of loading older data formats for may records, pulling old data into new places and translating as necessary. DovahKit is capable of doing the same thing in any situation where I RE'd said conversion. There is a problem, however: what the heck do we do when we *save* data?

Currently, the backend allows the frontend to request a desired record version. Forms can check the record version when saving, but can't influence it. The frontend always requests version 43 for Skyrim Classic and 44 for Skyrim Special (basing this on the output of `dovah::tes_file_writing::write_config` static member functions). Apparently, per code comments, I also intended for 0 to be a valid option, with it treated as "use the version number from the record we loaded," but that ~~doesn't make any sense since overrides, um, *exist*~~ isn't actually implemented and would actually output an actual zero as the record version numbers. Thankfully, nothing tries to use 0.

So it's already a bit messy, as we see.

There are other problems as well, however:

* Forms in DovahKit are currently only coded to *upgrade* data. They never(?) downgrade it: form-save code seldom checks for and handles older version numbers.
* Some entire subrecords are legacy data. For example, SOPM/ONAM replaces SOPM/CNAM and SOPM/SNAM, with the game and CK able to load the latter two for compatibility's sake. But what older form version are those two legacy subrecords *from*? When was ONAM introduced? We have no way of knowing.
* Not all changes in the versioned format are visible when reverse-engineering. If, for example, a data structure has grown longer, we cannot know its original (shorter) length in any past form version.

As such, we should do one of the following courses of action:

* Remove the ability to request any form version numbers before 43, throwing an exception if any is requested.
* Remove the ability to request a form version number at all, hardcoding the file save process to use, for all records, whatever form version corresponds to the game and file version number we're using.
