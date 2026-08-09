
# Asset paths

Skyrim's handling of asset paths is... highly variable. The game has several functions that transform paths in very different ways, but most of these functions appear to be completely unused.

## Known functions

### `AdvancePastDataDirectory`

**LE:** address 0x00A5A7F0  
**SE:** Address Library ID 69599

Checks if the input path starts with `"Data\\"` or `"data\\"`; if so, returns a pointer to the content just past that stem (i.e. `Arg1 + 5`). Otherwise, returns the input path verbatim.

Based on some observations about its callers, this seems to be part of the BSA internals? But I'm not sure if it's even used. I set a breakpoint on it in LE and got no hits. Most of the functions that can potentially call it are dead ends, i.e. if you follow the callers as far out as possible, you end up at functions that aren't referenced, aren't in a v-table, and don't have their addresses listed anywhere in the executable.

#### Translation from x86 to C++

```c++
const char* AdvancePathPastDataDirectory(const char* path) {
   if (path[0] == '\\')
      ++path;
   if (!_strncmp(path, "Data\\", 5))
      if (!_strncmp(path, "data\\", 5))
         return path;
   return path + 5;
}
```

### `NormalizeTexturePathSuffix`

**LE:** address 0x00AFDE00  
**SE:** unused copy at Address Library ID 75963; used copy is inlined into its sole caller, Address Library ID 75962

Similar to `NormalizeAssetPath` (2), except that instead of inserting a filename suffix, it *replaces* one if present and inserts it otherwise. The input path will be trashed after the call (i.e. some characters are clobbered with nulls, to facilitate the use of old-style C string library functions).

| Input Path | Suffix | Output | Output (with Data if missing) |
| :- | :- | :- |
| `"Data\\Foo\\bar"` | `_0` | `"Data\\Foo\\bar"` | `"Data\\Foo\\bar"` |
| `"Data\\Foo\\bar.txt"` | `_0` | `"Data\\Foo\\bar_0.txt"` | `"Data\\Foo\\bar_0.txt"` |
| `"Data\\Foo\\bar_1.txt"` | `_0` | `"Data\\Foo\\bar_0.txt"` | `"Data\\Foo\\bar_0.txt"` |
| `"Data\\Foo\\bar_a.txt"` | `_0` | `"Data\\Foo\\bar_0.txt"` | `"Data\\Foo\\bar_0.txt"` |
| `"Data\\Foo\\bar_a_b.txt"` | `_0` | `"Data\\Foo\\bar_a_0.txt"` | `"Data\\Foo\\bar_a_0.txt"` |

The exact same edge-cases apply to this function as to `NormalizeAssetPath` (2) with respect to file paths that appear absolute, and whether these functions consider those paths absolute.

This function's sole caller takes a single path as an argument. It loops over all possible texture suffixes, synthesizes a new path based on the input path and a given suffix, and passes that path to some unknown function. I assume this may have something to do with prefetching textures. However, the caller in question appears to be completely unreferenced, i.e. this is all dead code. The texture suffixes are stored as an array of string pointers, with the strings being:

* `_g`
* `_hh`
* `_hl`
* `_n`
* `_msn`
* `_s`

I set a breakpoint on this function in LE, and got no hits when starting up the game, COCing to Whiterun, equipping different armor, and COCing to Windhelm.

#### Translation from x86 to C++

```c++
void NormalizeTexturePathSuffix(
   char*       dst_path,
   char*       src_path,
   const char* filename_suffix
) {
   char extension[10]; // esp10
   char path_and_name[MAX_PATH]; // esp1C
   strcpy_s(path_and_name, sizeof(path_and_name), src_path);

   auto* ebx = dst_path;
   dst_path[0] = '\0';

   auto* last_dot   = strrchr(path_and_name, '.');  // ebp
   auto* last_sep   = strrchr(path_and_name, '\\'); // edi
   auto* src_suffix = strrchr(path_and_name, '_');  // esi
   if (last_sep && src_suffix && src_suffix < last_sep)
      src_suffix = nullptr;
   if (!last_dot)
      return;

   strcpy_s(extension, sizeof(extension), last_dot);
   if (src_suffix)
      *src_suffix = '\0';
   *last_dot = '\0';

   if (
      path_and_name[0] != '\\' &&
      path_and_name[1] != ':'  &&
      _strnicmp(path_and_name, "data", 4) != 0
   ) {
      strcpy_s(dst_path, MAX_PATH, "DATA\\");
      strcat_s(dst_path, MAX_PATH, path_and_name);
      strcat_s(dst_path, MAX_PATH, filename_suffix);
      strcat_s(dst_path, MAX_PATH, extension);
      return;
   }
   strcpy_s(dst_path, MAX_PATH, path_and_name);
   strcat_s(dst_path, MAX_PATH, filename_suffix);
   strcat_s(dst_path, MAX_PATH, extension);
}
```

### `NormalizeAssetPath` (1)

**LE:** address 0x00687F60  
**SSE:** inlined into callers: Address Library IDs 36133 and 36134, 

Copies the null-terminated source path to the destination, converting ASCII letters to uppercase and converting directory separators (`/` or `\`) to the preferred separator (`\`).

This function is used as part of the `BSResource` internals, with its results passed into `BSResource::CacheDrive::Op::Op(BSFixedString&, unknown_t)`. It also has other callers that I'm unable to identify (no nearby RTTI). When I breakpointed this function, I got no hits, at startup, when COCing from the title screen to Whiterun, when equipping armor, when speaking to an NPC, or when COCing to Windhelm.

#### Translation from x86 to C++

```c++
const char* NormalizeAssetPath(
   char*       dst_path,
   const char* src_path
) {
   const char* src_char = src_path; // edi
   char*       dst_char = dst_path; // esi
   bool        bl       = true;
   do {
      int32_t c = *src_char; // eax
      ++src_char;
      if (c > 0x7A)
         continue;
      if (c >= 'a' && c <= 'z') {
         c = toupper(c);
      } else if (c == '/' || c == '\\') {
         c = GetPreferredPathSeparator(); // { return '\\'; }
      } else if (c == '\0') {
         bl = false;
      }
      *dst_char = c;
   } while (++dst_char, bl);
   return dst_path;
}
```

### `NormalizeAssetPath` (2)

**LE:** address 0x00C70F80  
**SE:** function no longer exists?

If the path contains no `.`, then the function returns immediately; otherwise, everything after the last `.` is assumed to be a file extension. The function inserts a desired `filename_suffix` at the end of the filename, before the extension, and if the caller so desires, it'll also add a `"Data\\"` prefix if the path isn't absolute (i.e. does not start with a preferred directory separator, with `"Data\\"` (case-insensitive), or with an apparent drive letter).

| Input Path | Suffix | Output | Output (with Data if missing) |
| :- | :- | :- |
| `"Data\\Foo\\bar"` | `_0` | `"Data\\Foo\\bar"` | `"Data\\Foo\\bar"` |
| `"Data\\Foo\\bar.txt"` | `_0` | `"Data\\Foo\\bar_0.txt"` | `"Data\\Foo\\bar_0.txt"` |
| `"Foo\\bar.txt"` | `_0` | `"Foo\\bar_0.txt"` | `"Data\\Foo\\bar_0.txt"` |
| `"\\Foo\\bar.txt"` | `_0` | `"\\Foo\\bar_0.txt"` | `"\\Foo\\bar_0.txt"` |
| `"C:Foo\\bar.txt"` | `_0` | `"C:Foo\\bar_0.txt"` | `"C:Foo\\bar_0.txt"` |
| `"1:Foo\\bar.txt"` | `_0` | `"1:Foo\\bar_0.txt"` | `"1:Foo\\bar_0.txt"` |
| `"\\Data\\Foo\\bar.txt"` | `_0` | `"\\Data\\Foo\\bar_0.txt"` | `"\\Data\\Foo\\bar_0.txt"` |
| `"Foo\\bar.txt\foo"` | `_0` | `"Foo\\bar_0.txt\foo"` | `"Data\\Foo\\bar_0.txt\foo"` |
| `"Foo\\bar.txt\foo.txt"` | `_0` | `"Foo\\bar.txt\foo_0.txt"` | `"Data\\Foo\\bar.txt\foo_0.txt"` |
| `"Foo\\a.b.c.d"` | `_0` | `"Foo\\a.b.c_0.d"` | `"Data\\Foo\\a.b.c_0.d"` |
| `"Data/Foo\\bar.txt"` | `_0` | `"Data/Foo\\bar_0.txt"` | `"Data\\Data/Foo\\bar_0.txt"` |

This doesn't appear to have any callers in LE, and I can't even find an equivalent function in SE.

#### Translation from x86 to C++

```c++
void NormalizeAssetPath(
   char (&dst_path)[MAX_PATH],
   const char* src_path,
   const char* filename_suffix,
   bool        prepend_data_if_absent
) {
   char extension[10]; // esp08
   char path_and_name[MAX_PATH]; // esp14
   strcpy_s(path_and_name, sizeof(path_and_name), src_path);

   dst_path[0] = '\0';

   char* esi = strrchr(path_and_name, '.');
   if (!esi)
      return;
   strcpy_s(extension, sizeof(extension), esi);
   *esi = '\0';

   if (
      prepend_data_if_absent &&
      path_and_name[0] != '\\' &&
      path_and_name[1] != ':'  &&
      _strnicmp(path_and_name, "data", 4) != 0
   ) {
      snprintf(dst_path, MAX_PATH, "Data\\%s%s%s", path_and_name, filename_suffix, extension);
   } else {
      snprintf(dst_path, MAX_PATH, "%s%s%s", path_and_name, filename_suffix, extension);
   }
}
```


### `ScopePathToFolder`

**LE:** address 0x00A3F5C0  
**SSE:** Address Library ID 69839

Takes a buffer and size to write to, a path, and a desired folder name. If any non-trailing segment in that path is equal to the given folder name, returns a pointer to the start of said segment within the path. Otherwise, copies the folder name into the buffer, followed by the input path.

Logging and breakpoints indicate that this is used not only for paths in form data, but also paths in NIF files. This seems to be *the* primary function for correcting paths to be relative to the correct stem.

| Input Path | Input Folder | Output |
| :- | :- | :- |
| `"Data\\Sound\\foo"` | `"sound\\"` | `"sound\\foo"` |
| `"Sound\\foo"` | `"sound\\"` | `"sound\\foo"` |
| `"foo"` | `"sound\\"` | `"sound\\foo"` |
| `"Data\\Sound\\foo\\Sound\\bar"` | `"sound\\"` | `"sound\\foo\\Sound\\bar"` |
| `"BLARGH\\Sound\\foo"` | `"sound\\"` | `"sound\\foo"` |
| `"Data\\Sound\\"` | `"sound\\"` | `"sound\\"` |
| `"Data\\Sound"` | `"sound\\"` | `"sound\\Data\\Sound"` |

#### Known callers

* LE:4476B0 uses this to scope paths to `meshes\`
* LE:463D10 uses this to scope paths to `meshes\`. The caller is a member function on `TESObjectLAND` that has something to do with grass.
* LE:463F40 uses this to scope paths to `meshes\`
* LE:485C90 uses this to scope paths to `sound\`
* LE:486050 uses this to scope paths to `sound\`
* LE:4D6770 uses this to scope paths to `sound\`
* LE:7DA690 uses this to scope paths to `sound\`. The caller also manipulates DOBJ index 0x93, and calls `BSSoundHandle::Play`.
* LE:4FE2C0 appears to load subrecords for `BGSMusicSingleTrack`, and uses this to scope paths to `music\`.
* Used when loading `SNDR/ANAM[]`, to scope paths to the `sound\` folder.
* LE:5A2A70 uses this to scope paths to `meshes\`. The caller does something with FaceGen.
* LE:5A2B40 uses this to scope paths to `meshes\`. The caller does something with FaceGen.
* LE:5A2C70 uses this to scope paths to `meshes\`. The caller does something with FaceGen.
* LE:653C50 checks if a path starts with the exact case-insensitive string `"Data\\Sound\\"`. If so, it uses this function to scope the path to `sound\`; otherwise, `music\`. It only does this for paths whose file extensions are `.wav`, `.xwm`, or `.fuz` (all case-insensitive).
* LE:876600 uses this to scope the value of the INI setting `[General]sMainMenuMusic` to `music\`.
* LE:AF5030 uses this to scope paths to `meshes\`. Callers include `TESObject::Clone3D`.
* LE:AF54C0 uses this to scope paths to `meshes\`. Callers include `TESObject::Clone3D`.
* LE:AF5530 uses this to scope paths to `meshes\`. Callers include a member function on `TESNPC`, and `TESRace::FinalizeDataLoad`, the latter of which is called in a loop by `TESDataHandler` for every Race.
* LE:AF5680 uses this to scope paths to `meshes\`. The caller is involved in loading models, used for weapon nodes, skinned actor meshes, and possibly more.
* LE:AF58E0 uses this to scope paths to `meshes\`. Its callers include...
  * A member function on `TES` uses it for the paths contained in game settings `sBloodParticleDefault` and `sSplashParticles`.
  * A function that also checks the INI setting `[General]bUseBodyMorphs`.
* LE:B02B30 uses this to scope paths to `textures\`. Its callers include `BGSTextureSet::Load`.
* LE:B02C00 uses this to scope paths to `textures\`
* LE:B02C80 uses this to scope paths to `textures\`
* LE:B02DC0 uses this to scope paths to `textures\`
* LE:BA9CF0 uses this to scope paths to `meshes\`
* LE:BAA5A0 uses this to scope paths to `meshes\`
* LE:BAA6E0 uses this to scope paths to `meshes\`

#### Translation from x86 to C++

```c++
const char* ScopePathToFolder(
   char*       dst_path,
   size_t      dst_size,
   const char* path,
   const char* folder // should end with a directory separator
) {
   const char* existing_scope = nullptr; // ebp

   size_t ebx = strlen(folder);
   if (tolower(path[0]) == std::towlower(folder[0])) {
      if (_strnicmp(path, folder, ebx) == 0)
         existing_scope = path;
   } else {
      if (path[0]) {
         const char* path_ptr = path; // esi
         char        c        = *path_ptr;
         do {
            ++path_ptr;
            if (existing_scope)
               break;
            if (c == '/' || c == '\\') {
               if (ebx <= 0)
                  continue;
               if (_strnicmp(path_ptr, folder, ebx - 1) == 0) {
                  char d = path_ptr[ebx - 1];
                  if (d == '/' || d == '\\') {
                     existing_scope = path_ptr;
                  }
               }
            }
         } while (c = *path_ptr);
      }
   }

   if (existing_scope) {
      return existing_scope;
   } else {
      strcpy_s(dst_path, dst_size, folder);
      strcat_s(dst_path, dst_size, path);
      return dst_path;
   }
}
```
