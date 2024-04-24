
The process of saving an active file is somewhat complex, due to the need to support both Skyrim Classic and Skyrim Special, as well as the need to support converting files from either game to the other.

To start with, we need to weed out the obvious errors: there must be an active file; another save or load operation can't currently be in progress; if the active file is implicit, then it needs a filename; and the active file can't have enough masters to shove its forms into the 0xFF slot.

After that, we need to run a few additional checks centered around ESLs and cross-game conversion. ESL save operations need to fail if any form IDs are outside of the valid range for ESLs. Cross-game conversion needs to fail if the current load order is impossible in the target game: conversions from Classic to Special would fail if the number of masters is high enough for the full load order to overflow into the 0xFE slot; the reverse conversion would fail if the active file has any ESL masters.

Once we've decided that we can save the file, we need to perform several tasks, but the order in which we perform them is very important. The list, first without any consideration given to order:

* We need to write out data to a temporary \*.TES file.
* We need to close the active file's mapped view, so we can gain write access.
* We need to rename the \*.TES file to overwrite any \*.ES\* file that may be present.
* We need to toggle whether the load order supports ESLs, based on what game we're saving content for.
* If we're changing whether the active file is an ESL, then we need to renumber all of its forms in-memory, moving them from or to the 0xFE slot.
* We need to update the active file's in-memory header data, such as its list of dependencies.
* We need to attempt to reopen the active file's mapped view, so that editing can continue.
* We need to update the file offsets of all `form_stub`s for forms that are defined or overridden in the active file.

There are a lot of moving parts that we need to be mindful of. For example, if we are converting an ESL file to Skyrim Classic, we can't disable the load order's ESL support straightaway, as that would cause form ID prefix checks to fail: the checks would stop accounting for ESLs and thus treat active file form IDs as if they belong to the 255th file in the load order, since they're in slot 0xFE. Similarly, toggling ESL support would prevent us from getting the active file's load order prefix prior to the save operation, which would interfere with mass-renumbering forms in memory if that's necessary; as such, we need to make sure that we grab the active file's load order prefix (for use in distinguishing the active file's new forms from its overrides) beforehand.

In practice, the full save process is as follows:

* Check for obvious errors and fail if needed.
* If we're converting across games, check whether we can toggle ESL support and fail if we can't.
* Grab the active file's current `file_prefix`.
* Write to a temporary file.
* Toggle ESL support.
* Close the active file's mapped view, so that the existing file can be overwritten if need be (i.e. saving changes to a file for the same game), and so that we can open the view to a different file if need be (i.e. converting across games, or simply saving-as-new).
* Rename the temporary file, overwriting any existing file.
* If we changed whether the active file was an ESL, then mass renumber all of its forms. Use the `file_prefix` that we grabbed earlier to tell overrides apart from forms that are actually created in the active file.
* Update the active file's in-memory header data.
* Reopen the mapped view for the active file. If this fails, then tell the frontend that the save operation succeeded, but that editing cannot continue.
* If we were able to open the mapped file view, then update the file offsets on all `form_stub`s for forms defined or overridden in the active file. Delete any form stubs that were defined in the active file but didn't save, as well as any none-stubs that were referred to only by active file forms.

