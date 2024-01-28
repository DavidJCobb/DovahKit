
# BSA handling

## An overview of the BSA file format

**BSA** presumably stands for **Bethesda Softworks Archive**, and is basically a proprietary file format with a similar purpose and function to ZIP files.

BSAs consist of *folder entries* and *file entries*. A folder entry represents a single path, and is the parent of file entries. File entries, in turn, represent compressed files within the archive. Note that folder entries represent *paths*, not individual folders: they cannot be nested. The full path `abc\def\ghi` is one folder entry.

Folder and file entries are uniquely identified by an eight-byte hash. Although BSAs *can* embed folder and file names as strings, they are not required to do so; the hash is the only reliable identifier.


## Classes

### `bsa_archive`

Represents a single BSA archive.

### `bsa::packed_folder_info`

Info for a packed folder path.

### `bsa::packed_file_info`

Info for a packed file path.