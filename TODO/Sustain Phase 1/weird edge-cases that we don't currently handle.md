
# Weird edge-cases and behaviors that we don't currently handle

## All forms

### Deleted records break the Rule of One

The way the Rule of One typically works is that the game and CK will load a record and then, upon seeing an override for that form in a later file, manually clear the form's data and then load the override. Some forms coalesce data across multiple records, because clearing works via virtual member functions on the form, and some form types just choose not to clear everything.

However, if an override record is flagged as deleted, then *the form is not cleared* before loading that override.

DovahKit currently doesn't handle this edge-case. The vast majority of form types assume that data should only be loaded from the winning record, and check accordingly. Use info generation works similarly, and requires some jank to persist state across multiple records &mdash; jank that would have to be generalized to *every form type* in order for us to be able to change the form loaders to handle this.

Basically, if the last *n* records are flagged as deleted, then we need to coalesce the last *n + 1* records.

### Record flags during load

A typical implementation of the `TESForm::Load` virtual member function will begin by loading basic data from the record header, including the form ID and record flags. All previously-loaded flags are cleared save for the following, which appear to all be run-time state flags that shouldn't be considered valid on a serialized record:

* `1 << 14` (Temporary)
* `1 << 21` (Still Loading)
* `1 << 22` (Retains ID)

After the game calls `TESForm::Load`, it checks whether the current record comes from a master-flagged file and, if so, sets flag `1 << 0` (Is Master) on the form itself. Therefore, the form will have that flag set if the winning record comes from a master-flagged file.


## Form-type-specific edge cases

### Sibling landscapes override each other

If multiple `LAND` forms are children of the same `CELL`, then the later-loaded siblings will override the first-loaded sibling. This is because the logic for loading landscapes is special-cased as follows:

```c++
//
// Given:
//
//    TESFile* Arg1; // file to load from
//    TESForm* Arg3; // pre-existing form, if any
//
case form_type::landscape: // at 0x004409B9
   if (current_loaded_cell) {
      Arg3 = current_loaded_cell->GetOrCreateLandscape();
      if (!Arg3) {
         Arg3 = new TESObjectLAND;
         Arg3->SetParentCell(current_loaded_cell);
         current_loaded_cell->SetLandscape(Arg3);
      }
      LoadFormAndMaybeDoOtherStuff(Arg3, Arg1);
   }
   break;
```

I don't think we should *always* perform this overriding behavior. It should be an option that can be set before data is loaded. Implementing it would be a bit complicated:

* This has to be handled in the core of the backend, in the logic for accepting form stubs from a file loader and checking whether those stubs are overrides.
* If the affected cell and landscapes are originally defined in the active file, then we can just blindly coalesce things.
* If the affected cell and landscapes are originally defined in a non-active file, then we have to make sure that the form IDs for the override-via-sibling landscape records are filled by none-stubs, so that the end user can't inject other forms into that ID.

We also need to double-check which form ID the landscape ends up with (that of the earliest-loaded or latest-loaded sibling).
