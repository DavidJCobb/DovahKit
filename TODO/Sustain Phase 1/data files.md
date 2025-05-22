# File loading

The data structures for this feel like spaghetti. They're better than they used to be back in DovahKit's very early proof-of-concept stage, but they're still years old and poorly designed. Is there anything we can do to improve them further?

One major conceptual flaw I see (looking into a lot of this machinery on 4/27/2024 while trying to disentangle and rewrite the error handling) is that `file_loader` represents both something which *loads* a file and affords you functions to read from it as a stream, *and* the authoritative "owner" of the actual file data and metadata. (And describing that in reverse: its base class, `basic_reader`, could be a file *or* a view into a file, with this varying from subclass to subclass; and consequently `basic_reader` itself doesn't know how to get information, e.g. the name, about the file it's actually reading, because it doesn't know whether to "ask itself" or ask something else.) This feels wrong.

We should have a `dovah::tes::file` class which holds and "owns" the mapped file, and then all the various "loaders" should act like views into this single authoritative file object.

Some specific ideas, first written down on 4/28/2024:

* **`dovah::tes::file`** as a single TES file, with ownership of a mapped view, and with known information pulled from the header and stored as members. It should not contain or require an owning `file_load_order`. There should not be any functions for actually reading data from the mapped view (e.g. `read` and `unchecked_read`), nor should there be any other "stream" fields like a "current position." Outside code (e.g. `form_stub`s; load processes; etc.) would create a `file_view` (see below) to perform reads.

  * But how, then, do we read the header information so that the `file` can store and report information about itself? Potentially? Have it create and use a throwaway `file_view` (see below) internally.

* **`dovah::load_order_file`** as a subclass of `dovah::tes::file`, which adds an owning `file_load_order&` as well as runtime-specific flags (e.g. "is dummy file for hardcoded forms" and "is dummy file for none-stubs"). This is what a `file_load_order` would store and what `form_stub`s would use in their file list.

  * In the current implementation, `form_stub`s identify their owning load order *via* their files, in order to keep the stubs' memory footprint to a minimum since we have so many of the things around. This is also why we have "dummy files" in the first place: so that `form_stub`s that were never loaded from a "real" file can still reach their owning load order.

* **`dovah::tes::file_view`** as a view into any given `file`. This would be the counterpart to what is currently called `dovah::tes_file_reading::basic_reader`, with state for groups, records, and subrecords, and functions like `load_record_at` and `next_record_or_group`.

  * An advantage of making `file_view` wrap a `file` is that if we encounter a structurally malformed file (e.g. a suspicious record signature, or an ill-formed `XXXX` subrecord), we'll be able to access the file data (most pertinently its name) when we throw an error. Right now, `dovah::tes_file_reading::basic_reader` is actually incapable of providing the filename as diagnostic information in these cases: a `basic_reader` doesn't know if it's a file (`file_loader`) or a view into a file (any other subclass), and consequently, doesn't know how to find its way to *whatever file it's reading* to query that file's name.

  * Perhaps we could even template this and allow some compile-time configuration, e.g. a choice of whether to throw exceptions on invalid data or `assert` correctness instead. (Why would you ever want to `assert` that user-supplied data is well-formed? Because we crawl the files fairly completely during the initial file load, and after that, on-demand form data loads operate on the assumption that the file *definitely is* structurally valid and reads will never throw. Checks for e.g. record signature validity are skipped, and we assert instead of throwing, but this is conditioned on a run-time flag that gets set on the whole file post-file-load. In the new system, a `form_stub` would create disposable `file_view`s for loading each file, and so we may as well move the throw/assert choice to compile-time, no?)

    * We may even want to go the extra mile and have a `record_view`, so that stubs can create views on the stack without burning extra stack space on e.g. machinery to track GRUPs.

  * And as long as we're rebuilding the logic for reading file content, we may as well add support for endian-flipped files, conditioned behind a `constexpr const bool`. Bethesda themselves have this support: if the file header's signature reads as `4SET`, then they know that the file endianness doesn't match the system's native endianness, and they byteswap every value they read.

    * Why condition it behind a `constexpr bool`? Two reasons. First: if we rewrite the file load system, then we'll want to be able to compare benchmarks as directly to the old system as possible to ensure there's no notable perf hit. We'd want to disable any endian-flip support for these initial tests to ensure the branching and similar doesn't impact performance; then enable it later and measure the performance impact.

    * Endian-flip branches should probably be marked as `[[unlikely]]`.

    * We'd have to audit use info and form load code to ensure that no byte-stitching is done in either place, lest any endian-flipped data break there.

    * How would we test this? We don't have any endian-flipped files, and creating one would be very cumbersome.

* **`dovah::tes::record_reader`** and **`dovah::tes::subrecord_reader`** as the interfaces for reading records, i.e. the replacements for `dovah::tes_file_reading::record` and `dovah::tes_file_reading::subrecord`.

  * The `record_reader` interface shouldn't offer any functions for reading arbitrary data (i.e. no `read` or `unchecked_read` functions). Clients that are given access to a record should be required to obey the file structure (i.e. open, read, and close subrecords). There are internals for file parsing that require pulling data from a record, but those functions could be made internal or the relevant reads could otherwise be done manually.

    This would be an improvement over the current design, wherein form loaders, form use info builders, and any custom parses (e.g. the frontend caching subrecords of interest) can just choose not to obey the file structure -- to pluck arbitrary bytes out of a record without bothering to heed subrecord boundaries.

## Observations

* File flags versus record flags
  * TESV.exe only cares about three flags from the `TES4` record header: `master`, `optimized`, and `localized_string_table`; these are stored on the `TESFile`; the rest are discarded.
  * TESV.exe sets the `checked` flag on `TESFile` instances if they pass all of the following checks, though it's not clear when (or if) files even get checked:
    * File version is not too new (i.e. greater than 1.7)
    * For each master:
      * The master exists
      * File version of the master is not too new (i.e. greater than 1.7)
      * These checks pass for the master (i.e. recurse, and set the master as checked if appropriate (yes, this means they redundantly check the version twice))
  * The `active` flag is probably set on the active `TESFile` at run-time, by the Creation Kit.
  * It's not clear what the situation is with "internal" record/form flags, i.e. whether they ever get forcibly cleared somewhere. The flags at issue would be `1 << 0` (master) and `1 << 1` (altered), though `1 << 3` and `1 << 4` are also a mystery.
    * Form flag `1 << 2` is not a standard record flag and can be used by different form types.
    * Form flag `1 << 5` indicates a deleted record.
    * Form flag `1 << 7` is not a standard record flag and can be used by different form types.
    * Form flag `1 << 8` is not a standard record flag and can be used by different form types.
    * Form flag `1 << 12` indicates an ignored record.
      * "Ignored?" Like, in the CK's "load file" dialog, when you inspect a file and decline to load specific records?
      * **TODO:** The loader skips these. Do we?
        * The game's loader, or just the CK's loader?
    * Form flag `1 << 14` indicates a partial record.
    * Form flag `1 << 18` indicates a compressed record.
  * TESV.exe sets record flag 1 (matching `tes_file_flag::master`) on a form if any of its records come from a master (even if it's overridden by a non-master).
  * TESV.exe sets record flag 2 (matching `tes_file_flag::altered`) on a form if any of its records come from an `active`-flagged file (even if it's overridden by a non-active file).
  * That leaves two flags in `tes_file_flag` unexplained: `temp_id_owner` and `precalc_data_only`.

# `file_load_order`

Hm... I don't like its name and I don't like that it's stored in the `dovah/files/` directory.

I think `active_load_order` might be a better name. This would better distinguish it from the general concept of a "load order," while also matching the term "active file" and being clearer about the class's purpose: it holds all of the loaded data associated with a load order; it's the DovahKit counterpart to Bethesda's `TESDataHandler`. We could then repurpose the name "file load order" for the class that we currently call something like "load order normalizer."

One thing I'd really like to do is do a better job of separating out all the machinery related to loading and saving. It'd be nice if `active_load_order` would just retain the loaded data, and defer to temporary data structures for the actual load and save operations &mdash; perhaps something like `dovah::load_order_serialization::load_process` and `dovah::load_order_serialization::save_process`. Passkeys could grant them appropriate access to the `active_load_order` internals.

(PRE-LAUNCH UPDATE: We now use a `save_process`, but it's just a straight-up `friend` of multiple types; and it's used entirely within `file_load_order`, rather than being something you can create externally and then invoke on an active load order. Really, at present it's just a means of moving the save code to other files and splitting it up a bit for organization's sake, rather than being a true refactor. Plus, the machinery for handling files is also messy and so a lot of stuff is still spaghetti. A more complete redesign and rewrite would be needed, with careful consideration given to things like what information needs to be known by what systems, and when, for the purposes of things like error reporting.)

Miscellaneous:

* `form_creation_request::commit` and friends should return a `form_stub&`, so callers don't have to check whether the request succeeded even when it doesn't throw an exception.
* Improve `base_form_load_warning`: it should probably be possible to specify an alias ID when warnings occur while loading a quest alias.