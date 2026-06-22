
# Large references

Every worldspace maintains a `BGSLargeRefData`, which allows the worldspace to find "large references" based on the grid coordinates of a given cell. This is used to render these refs at full detail when their parent cells are unloaded but lie within the `uLargeRefLODGridSize` distance. (That distance is measured in cells and defaults to 9; `uGrids To Load` defaults to 5. The two grids shall here be referred to as the "large ref grid" and the "loaded cell grid," respectively.)

Per DynDOLOD documentation, if the large ref grid is equal to the size of the loaded cell grid, then the large ref system is turned off.

It appears that large refs have to be persistent. This makes sense: the game must already be able to find a `TESObjectREFR` by form ID and tell it when to load or unload its 3D.

## Data format

Large reference data is stored in `WRLD/RNAM` subrecords, one per cell. The serialized format is as follows:

* For each `RNAM`
  * The cell's grid coordinates (Y, then X, each as `uint16_t`s)
  * The number of large refs which overlap this cell (as a `uint32_t`)
  * For each such ref:
    * The ref's record ID
    * The grid coordinates for the ref's parent cell (Y, then X, each as `uint16_t`s). If, somehow, the ref has no parent cell, then its own position is converted to grid coordinates and used here.

This data is coalesced across all non-partial records, i.e. it does not follow the Rule of One.

The `RNAM` subrecord is only saved by the Creation Kit if: version control is enabled; and the current record isn't partial, *or* the file is being saved for a big-endian target (e.g. Xbox). If `RNAM` is to be saved, it is first rebuilt from scratch:

* All data is cleared.
* A list or map of candidate refs within the worldspace (all non-movable persistent refs?) is iterated. Refs that meet the appropriate criteria are re-added to the large reference data...
  * The ref must be "large."
  * The ref must not be flagged as "deleted."
  * The ref must overlap a non-zero number of cells.

### What refs may be saved, if they're "large?"

Unclear.

* Dawnguard.esm doesn't contain all of the same `RNAM` data as Skyrim.esm. It does contain refs that are defined in Skyrim.esm, but apparently only if those refs are overridden in Dawnguard.esm (even if only with ITMs).
* Sometimes, a cell lists the same ref multiple times in its [the cell's] `RNAM`. It's not altogether clear why, but it's possible that the worldspace's map of cells(?) to candidate refs can list the same ref multiple times (perhaps if overrides of the ref exist?). I doubt the duplicate ref entries within any single cell are *necessary*; I think the CK simply doesn't bother to avoid creating them.

Based on how the CK validates the large ref map, it seems like large refs must be persistent in order for them to be retrieved on-demand properly.

### Which refs are "large?"

The CK32 function at `6AE0A5` checks if a `TESObjectREFR*` is large. The criteria it checks are as follows:

First, the ref's base form must be a Static (or, in SSE, it may be a MovableStatic with the "Static" flag).

Next, the ref's computed half-extent must be greater than or equal to the `fLargeRefMinSize` game setting. The half-extent is computed as follows:

* If the ref has `ExtraPrimitive` data, then its computed half-extent is the primitive radius or X-axis half-extent.
* Otherwise:
  * Compute the *bounds magnitude* by accessing the `OBND` on the base form. Subtract the minimum coordinates (as `int16_t`s) from the maximum coordinates, compute the length (as a `float`) of the resulting vector, divide it by half, and return it.
  * Compute the *computed scale* of the ref by first retrieving its `ExtraScale` value as a float (normalized so that 100% scale is represented as 1.0). Then, if the ref is an Actor, retrieve the appropriate height based on its actor (i.e. race+sex height multiplier times per-actor height multiplier), and multiply that by the scale. The result is the computed scale.
  * The ref's computed half-extent is the bounds magnitude times the computed scale.

### What cells does a large ref overlap?

Begin by computing the ref's *computed half-extent* as described in the previous section.

Now, compute the minimum X-coordinate by taking the ref's X-position, and subtracting the computed half-extent. Compute the maximum X-coordinate by taking the ref's X-position, and adding the computed half-extent. Compute the minimum and maximum Y-coordinates similarly.

Shift all four coordinates right by 12 bits (equivalent to a division by 4096 for positive values; somewhat different, in important ways, for negative values) to get grid coordinates. Every cell within these coordinates (i.e. [min, max]) is considered to overlap the large ref.


## Known issues

According to DynDOLOD's documentation:

* Child worldspaces don't inherit large references from parent/ancestor worldspaces, even when they're set to inherit LOD in general.

* If a non-master file overrides a large reference, then the ref's full model will no longer be rendered when the ref is outside the loaded cell grid but within the large ref grid. Somehow, this also causes a problem wherein all other large references in the same cell will still render within that distance, but will fail to unload their generated LOD models, potentially causing visual overlap or Z-fighting.

* If a large ref (or its enable state parent) is Initially Disabled, then it may likewise render both its full model and its generated LOD model at the same time.

* If a pre-v44 data file overrides a `MSTT` form, clearing the movable-static's "Static" flag, then bugs may result. The documentation does not specify which bugs occur.

* If a large ref has its base form changed to a form that is not appropriate (i.e. not a Static or, in SSE, a Static-flagged MovableStatic), then bugs may result. The documentation does not specify which bugs occur.
