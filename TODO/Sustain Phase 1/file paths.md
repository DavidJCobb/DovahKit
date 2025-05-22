
File paths are handled very inconsistently throughout Skyrim's form data. Sometimes, they're fully-qualified paths, including the root `Data\` directory. Other times, they're relative to a top-level subfolder, e.g. `Data\Meshes\`.

The May 22, 2025 refactor of DovahKit's `DKGameFilePicker` improved our handling of file paths throughout the UI, but it still sucks; it's just *clear* now. A large part of the problem is that the handling of file paths is done purely within `DKGameFilePicker`; we have to translate paths from `ui::types::game_file_path` to `QString` to send them to other widgets, and paths in the backend are stored as `std::string` with no "semantic" information about the path format. `DKGameFilePicker` stores all paths as relative to and including `Data\`, so when form data stores a path relative to a subfolder, the UI code has to manually add and remove the prefix (e.g. `Data\Meshes\`) when moving paths in and out of the `DKGameFilePicker`.

Ideally, we should refactor things so that file paths are defined within the backend. Specifically, we should define two classes:

* **`dovah::asset_file_path`:** This would be the equivalent of the current `ui::types::game_file_path`: an analogue to `std::filesystem::path` but specialized for Bethesda's path scheme. These paths are either absolute or relative, with absolute paths treating `Data\` as the root.
  
  One major difference: `ui::types::game_file_path` in current DovahKit can't distinguish between "Data\" as an absolute path and "Data\" as a relative path. That is: ("Data\foo\bar\Data\").lexically_relative("Data\foo\bar\") produces an "absolute" path by mistake. The replacement `dovah::asset_file_path` shouldn't screw this up; internally, we should store the path as "\Data\foo\bar\Data\" versus "Data\", and just omit the leading slash when stringifying.

* **`dovah::rooted_asset_file_path`:** This is what would be present in form data. Essentially, it's a `std::string` that also specifies its root directory (e.g. "Data\Meshes\"), with an assignment operator that takes a `dovah::asset_file_path` and ensures that the appropriate root directory is in place.

So for example, our analogue to `TESModel` would store a `dovah::rooted_asset_file_path` with its root set to "Data\Meshes\". Then, the UI can just deal with `dovah::asset_file_path` and blindly assign that to the `TESModel`, without having to manually strip off the "Data\Meshes\" prefix.

There are some implementation details that would improve this idea further, by centralizing the relevant prefixes (so I'm not typing "Data\Meshes\" everywhere):

* Use an enum instead of storing arbitrary rooted paths. This would keep in-memory sizes down and prevent potential typos.
* Use typedefs or templating, e.g. `dovah::rooted_asset_file_path` as a superclass or template, and `dovah::mesh_file_path` as a subclass or template instantiation.