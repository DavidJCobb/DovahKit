
# Localized strings

For loaded state, ownership hierarchy is:

* **`active_load_order`**
  * **`localization::string_collection`\:** Top-level collection of all localized strings.
    * **`localization::string_file_bundle`\:** Holds all localized string content for a single (non-active) TES file.
    * **`localization::active_string_file_bundle`\:** Holds all localized string content for the active TES file.

## Localized string use info

A `string_collection` owns `use_node` instances. A `use_node` sits between used strings and their users:

* Given a `string_collection`, you can look up the `use_node` for an L-string (by associated TES file, string type, and string ID), and then from the `use_node`, you can reach the users.

  * Form stubs can be reached direectly via pointer.

  * Game settings are listed by name (since `loaded_game_setting`s are not heap-allocated, currently). You will have to look them up by getting the `string_collection`'s owning `active_load_order` and querying its `game_setting_collection`.

* Given a user `form_stub` or `loaded_game_setting`, you can look up the `use_node`s associated with that user, to get the identifying information for an L-string. Then, you can look that L-string up from the `string_collection`.
