
# Use Info

There are currently two kinds of use info: uses of forms, and uses of localized strings. These are tracked through two separate mechanisms.

## Form use info

Every `form_stub` maintains a map of outbound uses (i.e. "I use these other forms"), and a map of inbound uses (i.e. "these other forms use me"). This use info is built as part of the overall process of indexing a file into form stubs: we multi-thread building the outbound use info, and then use a single thread to mirror that for the inbound use info.

(These maps use form IDs as keys, rather than pointers. This is so we can more easily handle dangling uses: if a form claims to use a record ID that doesn't resolve to an existing form, then we store a use info entry sans pointer. Afterwards, we'll create "none-stubs" which reserve the form IDs that these dangling uses point to.)

The uses themselves are represented as `managed_form_use`s within a form's loaded data.

## String use info

If a file is flagged as using localized strings, then those strings may be used by forms or by game settings. These uses are tracked via `localization::use_node` structs: these are owned by the load order's `string_collection`, and serve as a bridge between using entities and used strings.

Form stubs maintain an optional vector of pointers to use nodes, while `loaded_game_setting`s maintain a pointer to a single use node (because we only retain a game setting's last loaded value, so in essence a string-type game setting can only be using one string). The use nodes, in turn, refer back to the using stubs and game settings.

The reason form stubs and game settings don't point directly to the used strings (as is done for form-to-form uses) is because they can't:

* A `string_file_bundle` holds all strings for a non-active TES file. These strings are held as offset/size pairs and are not individually heap-allocated, so although these bundles shouldn't be mutated after they're loaded, I'm still not sure it's the best idea to let anything point directly to those pairs.

* An `active_string_file_bundle` holds all strings for the active TES file. These strings are held as `std::string`s inside of unordered maps, and these also aren't individually heap-allocated. Since I intend for localized strings to eventually be alterable in-editor, this means that edits may invalidate iterators and move the `std::string`s around in memory. Thus, pointers to them wouldn't be stable.

* We can't guarantee that L-string content will ever even be loaded. If the `active_load_order` isn't specifically directed to load BSA archives, or if the string files associated with a given TES file are missing or out of date, there may not even *be* any l-strings for form stubs or game settings to hold pointers to.

(A similar limitation is that `loaded_game_setting`s also aren't individually heap-allocated, so the `localization::use_node`s have to track them by holding a `std::set` of the using settings' (case-folded) names.)

Theoretically, form-to-form uses could be tracked similarly (i.e. via shared "nodes," such that a user-form's "outbound" map and a used-form's "inbound" map hold a pointer to the same node), but this would require the nodes in question to be individually heap-allocated. That feels wasteful to me, given that the `std::map` will itself consist of a tree of heap-allocated nodes.