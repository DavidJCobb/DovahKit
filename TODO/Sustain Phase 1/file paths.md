
File paths are stored inconsistently within Skyrim's form data. Sometimes, they're fully-qualified paths, including the root `Data\` directory. Other times, they're relative to a top-level subfolder, e.g. `Data\Music\` or `\Data\Music\`. It seems like the game normalizes all file paths (in game data, NIFs, etc.) using a single function which essentially: checks for a path segment equal to a desired top-level folder name; and either removes any prefix prior to that folder, or appends the folder *as* a prefix.

(So for example, given the desired prefix `sound\`: `Data\sound\foo` becomes `sound\foo`; `sound\foo` is unchanged; `foo` becomes `sound\foo`; and `rofl\lmao\sound\foo\sound` becomes `sound\foo\sound`.)

The May 22, 2025 refactor of DovahKit's `DKGameFilePicker` improved our handling of file paths throughout the UI, but it still sucks; it's just *clear* now. A large part of the problem is that the handling of file paths is done purely within `DKGameFilePicker`; we have to translate paths from `ui::types::game_file_path` to `QString` to send them to other widgets, and paths in the backend are stored as `std::string` with no "semantic" information about the path format. `DKGameFilePicker` stores all paths as relative to and including `Data\`, so when form data stores a path relative to a subfolder, the UI code has to manually add and remove the prefix (e.g. `Data\Meshes\`) when moving paths in and out of the `DKGameFilePicker`.

Ideally, we should refactor things so that file paths are defined within the backend. Specifically, we should define the following types:

* **`dovah::asset_folder`:** An enum matching all top-level asset folders that the game scopes paths to (i.e. `meshes`, `music`, `sound`, `textures`), along with a `none` option.

* **`dovah::asset_file_path`:** This would be the equivalent of the current `ui::types::game_file_path`: an analogue to `std::filesystem::path` but specialized for Bethesda's path scheme. These paths are either absolute or relative, with absolute paths treating `Data\` as the root. These paths always normalize directory separators (i.e. backslashes only; no duplicates).
  
  One major difference: `ui::types::game_file_path` in current DovahKit can't distinguish between "Data\" as an absolute path and "Data\" as a relative path. That is: ("Data\foo\bar\Data\").lexically_relative("Data\foo\bar\") produces an "absolute" path by mistake. The replacement `dovah::asset_file_path` shouldn't screw this up; internally, we should store the path as "\Data\foo\bar\Data\" versus "Data\", and just omit the leading slash when stringifying. (That said, it is acceptable -- desirable, in fact -- if constructing a path from the *string* `"Data\\foo\\bar"` produces an absolute path.)
  
  Moreover, `dovah::asset_file_path` should explicitly use `std::string` as its underlying storage, not `QString`. The defined character encoding for paths in Skyrim is ASCII; non-null non-ASCII characters are effectively undefined; a Cyrillic Capital Letter De in Windows-1251 and a Latin Capital Letter A With Diaeresis in Windows-1252 are both code point 0xC4 and are therefore considered the same symbol with respect to path handling. `QString` should only be used to represent paths for display within the UI; if the user types in a path, it should be converted from whatever OS/display encoding is in use, to the single-byte backend encoding, as soon as possible (warning the user about non-ASCII characters).
  
  * Member function: `bool is_inside(dovah::asset_folder) const`.
  
  * Member function: `asset_file_path lexically_relative(dovah::asset_folder) const`.
  
  * Static member function: `asset_file_path from_within_asset_folder(dovah::asset_folder, std::string_view)`. The inverse of the `dovah::asset_folder` overload for `lexically_relative`.

* **`dovah::verbatim_asset_file_path<dovah::asset_folder::sound>` and similar:** This would reflect a file path loaded verbatim from saved content. Using the `sound` folder as an example, this path would be allowed to start with `\Data\sound\`, `Data\sound\`, `sound\`, or no prefix at all. (If the path starts with the wrong folder, that is treated as it having no prefix.) This is the type that would be used in form data, so that we correctly preserve whether or not a loaded file path has a stem.

  * Conversions from `dovah::asset_file_path` will throw if the asset file path doesn't start with `\Data\sound\`.

  * Conversions from e.g. `dovah::verbatim_asset_file_path<dovah::asset_folder::sound>` to `dovah::asset_file_path` will convert the prefix to `\Data\sound\` using the reverse of the logic the game uses to scope a loaded path to the top-level `sound\` folder:
  
    * `\Data\sound\foo` is unchanged
    * `Data\sound\foo` becomes `\Data\sound\foo`
    * `sound\foo` becomes `\Data\sound\foo`
    * `\Data\meshes\foo` becomes `\Data\sound\Data\meshes\foo`
    * `\meshes\foo` becomes `\Data\sound\meshes\foo`

  * We may benefit from defining type aliases for the various specializations, e.g. `using dovah::verbatim_mesh_file_path = dovah::verbatim_asset_file_path<dovah::asset_folder::meshes>`.

Form data would use verbatim asset paths. DovahKit's frontned would generally deal in `asset_file_path`s for asset loading, file selection by a user, and so on.
