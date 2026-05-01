
# Save process

Right now, the backend can only manage saving a game data file (ESP/ESM/ESL). It can't manage saving multiple accessory files, which is problematic for SEQ files and intractable for future localized string support.

We need to be able to deal with:

* Saving multiple accessory files
* Automatically deleting accessory files that become empty (e.g. SEQ)
  * Should be an option the frontend can specify, and in turn, an option presented in the UI.
* Dealing with a situation where some files can't be saved
* Dealing with a situation where all files are saved to temporary output, but some files can't be moved from their temporary files to their desired locations and names

We should also be more consistent with temporary files' names (when we write to a temporary file, confirm the write finished, and then replace the target file with the temporary): we should use `foo.dovahkit-bar` given a desired filename `foo` and a desired extension `bar`.