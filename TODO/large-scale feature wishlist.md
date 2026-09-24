
# Large-scale feature wishlist

## Better cross-game conversion

When converting a file across games, we don't check whether its masters also exist cross-game. We also don't check whether any forms overridden in the active file still exist in the masters for the destination game.

## Direct upload to NexusMods

The Creation Kit can upload directly to Bethesda.net. I've had someone suggest a comparable upload for NexusMods.

## Mod merging

Mod merging could be useful, particularly if we write a co-save file that describes the mapping of forms from the source files to the destination file (so that later re-merges produce consistent results).

Being able to selectively merge in content would also be helpful.

## Overlapped cell handling

If two cells in the same worldspace have the same grid coordinate, we should offer the user some means to resolve this (e.g. move one of the cells; discard one of the cells). At the very least, it should error on file load. I think right now, we don't handle this case at all, so among other things, the Render Window and whatnot would show whichever cell is found first when searching for the cell at a given grid coordinate.

## Override record features

The following features could be useful:

* **Reset form:** Take a form that is overridden in the active file, and remove that override, restoring the form to the state it was in previously.

* **Revert form:** Take any losing override of a form, and copy the content of that override into the active file (i.e. deliberately ITM a record other than the form's current winning override).

* **View revision:** Get a list of all records for a form (base and overrides, including losing overrides) and view the form as it would've existed at a given record.

"Reset form" and "Revert form" are tricky bordering on impossible in DovahKit's current (alpha) design. Use Info is originally built as part of the overall file load process, so for "Reset form," we'd have to be able to clear all Use Info for a form stub, remove the active file from the form stub's source file list, and then rebuild Use Info for that form stub individually. For "Revert form," we'd need the ability to load unmanaged form data only up to a given source file. Neither functionality currently exists in DovahKit.
