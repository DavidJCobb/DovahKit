
## Background

Versions of Skyrim's file format prior to 1.71 are subject to a bug in how record IDs are converted to form IDs when files are loaded. The game accidentally ignores the load order prefix when checking whether the record ID refers to a hardcoded form; therefore all record IDs below `xx000800` are loaded as overrides are hardcoded forms.

File format version 1.71, introduced during Skyrim Special Edition's lifetime, fixes this bug, allowing forms to have record IDs in the range [`xxyyy000`, `xxyyy7FF`]. This doubles the number of form IDs available for light plug-ins to use.

## Definitions

### `hardcoded_form_ids_always_ignore_record_id_prefix`

Given a `game`, tests whether that game is always subject to the above-described bug, such that any record IDs below `xx000800` cannot be used for new forms. This will be true for any game prior to Skyrim Special.

### `hardcoded_form_ids_ignore_record_id_prefix_until_file_version`

Given a `game`, returns a `std::optional<float>` indicating the minimum file version at which the bug is fixed, i.e. the minimum file version that can use record IDs in the range [`xxyyy000`, `xxyyy7FF`] for new forms. If the given `game` never had the bug fixed, returns an empty optional.

### `hardcoded_form_ids_ignore_record_id_prefix`

Given a `game` and a file version, returns a boolean indicating whether the above-described bug still occurs, i.e. returns `true` if the given game and file version cannot use any record IDs below `xx000800` for new forms.
