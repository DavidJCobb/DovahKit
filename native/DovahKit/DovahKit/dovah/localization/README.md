
# Localized strings

For loaded state, ownership hierarchy is:

* **`active_load_order`**
  * **`localization::string_collection`\:** Top-level collection of all localized strings.
    * **`localization::string_file_bundle`\:** Holds all localized string content for a single (non-active) TES file.
    * **`localization::active_string_file_bundle`\:** Holds all localized string content for the active TES file.
