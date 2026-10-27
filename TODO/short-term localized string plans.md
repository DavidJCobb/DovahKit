
## Things I want or need for localized string editing and use info

* Use Info for localized strings, to see where any given string is used

  * Can be read-only for non-active-file localized strings

* The ability to edit active file strings

  * Use Info, to know what strings can be deleted
  
  * The ability to switch whether the active file uses localized strings (with forms and GMSTs referring by string ID) or non-localized strings (with forms and GMSTs storing data inline as an `std::string`)
  
  * The ability to edit the text of an existing string, per language
  
  * The ability to create a new string
  
    * Including the ability to take a string used in two places, and split it into two identical (and independently alterable) strings each used in just the one place
  
## Things we have and things we need

### Have

* `string_file_bundle`, as read-only storage for non-active-file localized strings

### Need

* A data structure to store localized strings (mapping IDs to definitions, per language) for the active file

  * Code to load L-string files not into immutable buffers, but into this data structure (i.e. for the active file exclusively, we need to load localized strings (if any are present) the way the alpha used to; though instead of `std::string`, we can potentially use `lu::low_footprint_string`).

* A process whereby an active file can be switched between using localized and non-localized strings.

  * Non-localized to localized: We must iterate over every form type that can contain l-strings, iterate over every form of those types, pull the forms' string data into our centralized collection, and then write IDs back into the forms. Then, we must set the "localized" flag on the active file's header.
    
    * Strings should be de-duplicated: if identical text appears for multiple l-string fields (that are of the same l-string type, i.e. STRINGS/DLSTRINGS/ILSTRINGS), then we should create just one l-string ID and have it be shared across those fields.
  
    * We must do the same for string-type GMSTs.
    
    * We must also build l-string Use Info for the active file.
  
  * Localized to non-localized: User must specify a target language, and that language must be present. We must iterate over every localized string ID (taking advantage of l-string Use Info, as described below), load the using forms and GMSTs, and replace their ID-uses with inlined `std::string`s. Then, we must clear the "localized" flag on the active file's header.
    
    * We must remember to update l-string Use Info for the active file (i.e. by clearing it en masse).

* Form/l-string Use Info

  * Must be built at the same time as form-to-form Use Info
  
    * REMEMBER: We need typed IDs for the `tes::bytereader`, because there are three ID spaces for l-strings (STRINGS/DLSTRINGS/ILSTRINGS)
  
  * User-to-used info is ideally stored on the `form_stub`
  
  * Used-to-user info is ideally stored on the `string_file_bundle` (and the active-file counterpart, once I implement that) as a whole

* GMST/l-string Use Info

  * Must be built when loading GMST records
  
  * User-to-used info is ideally stored on the `loaded_game_setting::per_file_data` content
  
  * Used-to-user info is ideally stored on the `string_file_bundle` (and the active-file counterpart, once I implement that) as a whole

* When a form or GMST is deleted from the active file, we must sever Use Info between it and any l-strings it used.

* When an active file l-string is deleted, we must sever Use Info between it and any forms and GMSTs that used it, amending those to use ID 0 (i.e. "no string") in its place.

* Helpers, to enable robust editing in the future

  * On `string_file_bundle` and its active-file counterpart, a "for-each" member function template which takes a language and a `localized_string_type`, and permits const access to all defined string IDs and content. Bonus points if the string's use info is easily accessible as well. We want this so that users can browse the l-strings associated with a given TES-file and language. (Potentially the UI could build its own `QAbstractItemModel` and facilitate searching/filtering too.)

## What do we have to work with

* `form_stub` has 16 bytes to spare for tracking l-string use info; that should be enough for either a simple container (e.g. data pointer + length), or a more robust heap-allocated container (e.g. `std::unique_ptr<std::vector<some_l_string_use_info_type>>`

  * Another option is two pointers: one to non-active-file l-string use info; and one to active-file l-string use info. The former would be considered immutable after the initial use-info build step (because non-active files should not be considered modifiable); the latter would be more likely to mutate post-load. Each could use a different heap-allocated container to account for these differing properties.
  
    * Tempting to make the non-active use info just be a single pointer to some null-terminated list, but remember that we need to build this stuff incrementally file-by-file a.k.a. record-by-record as the form is being loaded; so for a form overridden by multiple localized non-active files, rebuilding a null-terminated list would be inconvenient (we'd have to measure its length `strlen`-style for every non-active-file record we parse, to know its existing size and know how to realloc it).
