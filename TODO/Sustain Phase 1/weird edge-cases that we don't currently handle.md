
# Weird edge-cases and behaviors that we don't currently handle

## All forms

### Deleted records break the Rule of One

The way the Rule of One typically works is that the game and CK will load a record and then, upon seeing an override for that form in a later file, manually clear the form's data and then load the override. Some forms coalesce data across multiple records, because clearing works via virtual member functions on the form, and some form types just choose not to clear everything.

However, if an override record is flagged as deleted, then *the form is not cleared* before loading that override.

DovahKit currently doesn't handle this edge-case. The vast majority of form types assume that data should only be loaded from the winning record, and check accordingly. Use info generation works similarly, and requires some jank to persist state across multiple records &mdash; jank that would have to be generalized to *every form type* in order for us to be able to change the form loaders to handle this.

Basically, if the last *n* records are flagged as deleted, then we need to coalesce the last *n + 1* records.

#### Partial parent forms break the Rule of One too

If a parent form (`CELL`, `DIAL`, `WRLD`) has an override flagged as "partial," it also skips the "clear" functions. Additionally, forms load full and "partial" data differently (`LoadForm` versus `LoadPartial`). This means that setting *or clearing* the "partial" flag on these forms after data has been loaded will have consequences that DovahKit is currently unable to handle:

* Setting the flag on the active-file record means that the form's contents need to be reset to match whatever record preceded the active file. It also means that the active file will no longer be able to set or override certain data (anything not loaded by `LoadPartial`). This situation is comparable to the jank we see with TopicInfos, wherein the "partial" flag causes some form data to effectively be bifurcated between the active file and its preceding files.

  By implication, our loaded use info will immediately become out of date, and will need to be rebuilt. We have no systems which would facilitate doing this.

* Clearing the flag on the active-file record would have the opposite effect: any data that originally came from masters should either be cleared, or moved into the active-file data. If setting the flag risks bifurcating data, then clearing the flag reunites all the data.

There are potential additional complications with all of this. I've seen some users report that if a cell's winning record is "partial," then the cell's contained refs may fail to load in-game. I'd need to investigate how cell loading works in order to know what's going on here.

There's no single unified system we can build to handle all of this. We'd have to extensively document the full versus partial load behavior for the affected form types, and give each form type a function that can handle the "partial" flag being changed. This function would need to be able to process data from arbitrary records preceding the winning record. The most we can do declaratively, I think, is give each form type a flag indicating whether it needs these behaviors.

### Record flags during load

A typical implementation of the `TESForm::Load` virtual member function will begin by loading basic data from the record header, including the form ID and record flags. All previously-loaded flags are cleared save for the following, which appear to all be run-time state flags that shouldn't be considered valid on a serialized record:

* `1 << 14` (Temporary)
* `1 << 21` (Still Loading)
* `1 << 22` (Retains ID)

After the game calls `TESForm::Load`, it checks whether the current record comes from a master-flagged file and, if so, sets flag `1 << 0` (Is Master) on the form itself. Therefore, the form will have that flag set if the winning record comes from a master-flagged file.


## Form-type-specific edge cases

### Placed projectile forms

**BLUF:** Rename the `dovah::form_type` constants for placed projectile forms. When designing editing features, e.g. Render Window features, be mindful that REFR subclasses are not interchangeable: you should not allow: an `ACHR` to have a base form that isn't an actor; nor a `PHZD` to have a base form that isnt' a hazard; nor a `PGRE` or similar to have a base form that isn't a projectile; nor a plain `REFR` to have a base form that *is* an actor, projectile, or hazard.

These forms are:

| Signature | Name | Constant | Notes |
| :- | :- | :- | :- |
| PARW | PlacedArrow | `arrow` | A fired arrow. |
| PBAR | PlacedBarrier | `barrier` |
| PBEA | PlacedBeam | `beam` |
| PCON | PlacedCone | `cone` |
| PFLA | PlacedFlame | `flame` |
| PGRE | PlacedGrenade | `grenade` | A thrown grenade or armed landmine. Note that the traps created by rune spells count as landmines. |
| PHZD | PlacedHazard | `placed_hazard` |
| PMIS | PlacedMissile | `missile` |

There's one obvious change we should make: all of the "placed projectile" form type constants should be renamed to be explicit about what the forms are.

* `placed_projectile_arrow`
* `placed_projectile_barrier`
* `placed_projectile_beam`
* `placed_projectile_cone`
* `placed_projectile_flame`
* `placed_projectile_grenade`
* `placed_projectile_missile`

Additionally, however, we currently define dummy loaded-form classes for these that just subclass `ObjectReference` while adding nothing and retaining all of its behaviors. This is sufficient for now, but it invites the possibility for jank in the future. What if, in the Render Window, someone uses the "change this ref's base form" feature to change a ref between a projectile and non-projectile base? (For that matter, what if they do the same for a placed `Actor`?)

It'd be nice if we could have all of these form types map to the `ObjectReference` loaded-form class, and have some means of deciding, at load and save time, what form type, signature, etc., to save the form with. In particular, it'd be nice if the `ObjectReference` loader could check the signature that the record has, in order to validate that the base form is of the correct type.

The problem with doing things that way, of course, is that Bethesda's form-type-override checks (to ensure that you don't override a form with data of the wrong type) don't allow reference types to be interchangeable. If something is defined as a `PHZD`, you cannot override it with a plain `REFR`, much less a `PMIS` or `ACHR` or something else. This means that we just have to be restrictive:

* Continue using separate loaded-form classes for each of these ref form types.

* If a ref is defined outside the active file, don't allow changing its base form to a category that would require a different ref form type.

* If a ref is defined inside the active file, allow these cross-form-type changes, but warn the user and ask them to confirm before proceeding. (If their active file is some other mod's master, then they'll break any incoming overrides of the ref from that other mod.)


### Reference overrides and persistence oddities
Refer to DovahKit's per-form documentation for refs. Basically, if a ref is defined across multiple records, and any of those records are flagged as persistent, then the non-persistent records are never retained or loaded. Note that records defined in a (non-master-flagged) ESP file are <i>per se</i> persistent.


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


## Editor logistics

### Converting files across games

* When we convert an active file across games, we check whether its masters exist in the destination game's Data directory, but we don't check that those destination-game masters have equivalent content (or, more generally, that the active file would properly override those masters without errors).

* When we convert an active file across games, we continue loading data from the source-game masters, rather than loading data from the destination-game masters. This is necessary if the user decides to save despite missing masters in the destination game, and it'd be necessary if the destination game's masters are incompatible (see previous bullet point); but even if the destination-game masters are correct, we still load data from the source-game masters until such time as the user reloads all involved files from scratch.

  This is obviously more efficient, compared to having to do a full reload, but it *does* give me pause, a little bit.
