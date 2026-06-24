
# Large references

Every worldspace maintains a `BGSLargeRefData`, which allows the worldspace to find "large references" based on the grid coordinates of a given cell. This is used to render these refs at full detail when their parent cells are unloaded but lie within the `uLargeRefLODGridSize` distance. (That distance is measured in cells and defaults to 9; `uGrids To Load` defaults to 5. The two grids shall here be referred to as the "large ref grid" and the "loaded cell grid," respectively.)

Per DynDOLOD documentation, if the large ref grid is equal to the size of the loaded cell grid, then the large ref system is turned off.

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

Large refs don't need to be persistent; there are several in the base game that lack the flag.

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

### Data format and generation

The Creation Kit uses a very quick-and-dirty way to measure out what cells are overlapped by a large reference. This has a few consequences:

* The size of a base form is measured by taking the distance from the pivot to the "min" corner, and the distance form the distance to the "max" corner, and averaging them. The result (further multiplied by the ref's scale factor) is treated as the half-width of a square area that the reference is assumed to overlap. Thus, objects that are significantly longer on one side may produce inaccurate overlap results.

* Additionally, even though the goal is to measure the lateral area that the large ref overlaps, the above calculation is influenced by the base form's bounding box height, so especially tall refs may end up with too large of a half-width, while relatively squat refs may end up with too small of a half-width.

* Even without the above bullet points, oblong large refs may produce incorrect results because refs' rotations are not considered.

It strikes me, though, that I'm not sure how (or *if*) the large ref system copes with a large ref being moved across cells (or even worlds) or changing in size. Large refs are associated with cells; there doesn't appear to be a way for overrides to *dissociate* them with those cells. Per CommonLibSSE, `BGSLargeRefData` stores three maps:

* A map. The key type is a cell's grid coordinates; the value type is an array of the form IDs of all large refs overlapping that cell.
* A map of large ref form IDs to their parent cells' grid coordinates.
* A map. The key type is a cell's grid coordinates; the value type is an array of the form DIs of all large refs that are explicitly contained in that cell. (Or rather, this may be a map based on `RNAM` entries wherein the "key" cell and the ref's parent cell are the same; so, if a large ref is moved from one cell to another, this map may associate it with both cells?) Per CommonLibSSE, it is this specific map that is used to load large refs on cell attach.

I'm not aware of any code that would remove a ref from any of these maps during load, though granted, the disassembly of `BSTHashSet` operations can be a nightmare to read...

### Run-time bugs

According to [DynDOLOD's documentation](https://dyndolod.info/Help/Large-References), there are some engine bugs in how large refs are handled. Some of the minor issues listed are:

* Child worldspaces don't inherit large references from parent/ancestor worldspaces, even when they're set to inherit LOD in general.

There are also bugs that can cause large refs to be rendered twice: once as LOD, and once as their full models.

* If a non-master file overrides a large reference, then the ref's full model will no longer be rendered when the ref is outside the loaded cell grid but within the large ref grid. Somehow, this also causes a problem wherein all other large references in the same area will still render within that distance, but will fail to unload their generated LOD models, potentially causing visual overlap or Z-fighting.

* If a large ref (or its enable state parent) is Initially Disabled, then it may likewise render both its full model and its generated LOD model at the same time.

* Per Sheson's[^dyndolod-guy] work on xEdit's "Generate Large References" script, [large refs should not be persistent or flagged as "full LOD"](https://github.com/TES5Edit/TES5Edit/blob/9058a79437367c2cb1b757a9021ab443d743012d/Build/Edit%20Scripts/Skyrim%20SE%20-%20Generate%20Large%20References.pas#L79) because they never unload. (I assume this means the ref would render at full detail even far outside of the large ref grid, when the ref's LOD is also being rendered.)

[^dyndolod-guy]: The creator and maintainer of DynDOLOD.

And some unspecified bugs include:

* If a pre-v44 data file overrides a `MSTT` form, clearing the movable-static's "Static" flag, then bugs may result. The documentation does not specify which bugs occur.

* If a large ref has its base form changed to a form that is not appropriate (i.e. not a Static or, in SSE, a Static-flagged MovableStatic), then bugs may result. The documentation does not specify which bugs occur.

#### Doubly-rendered refs

The reason large refs may render twice has to do with how the game bakes worldspace LOD. There are two kinds of LOD models:

* Files with the path <code>meshes/terrain/<var>worldspace</var>/<var>worldspace</var>.<var>size</var>.<var>x</var>.<var>y</var>.btr</code> are baked LOD meshes for landscapes and cell water planes: they're just normal NIFs with the file extension changed.

  * `worldspace` is the editor ID of the worldspace.

  * `size` refers to the number of cells covered by this mesh. Values seen are 4, 8, and 16, with INI settings existing to enable the use of size 32 if present. LOD meshes that cover larger sizes will use less detailed meshes and textures. As you approach an area, the smaller-area LOD meshes will be swapped in.

  * `x` and `y` are the grid coordinates of a corner cell (i.e. the mesh covers cells `x` up to, but not including, `x + size`, and thus also for `y`).

* Files with the path <code>meshes/terrain/<var>worldspace</var>/<var>worldspace</var>.<var>size</var>.<var>x</var>.<var>y</var>.bto</code> are baked LOD meshes for static refs placed in cells. They, too, are NIFs with a changed file extension.

Size-4 `bto` files are the only files I've found that go out of their way to divide the baked LOD for large refs into different shapes.[^large-ref-lod-shapes] These shapes use block type `BSSubIndexTriShape` (SSE) or `BSSegmentedTriShape` (LE), which defines a list of `BSGeometrySegment`s identifying a range of triangles (start index and count) belonging to the segment. It's not clear, however, how these segments correspond to the large refs. I've done some reverse-engineering, but haven't yet tracked down where or how the game turns segments off, or, in particular, how it even ties a given mesh segment to a given large ref.

[^large-ref-lod-shapes]: Names seen for SSE `bto` files are `objsnow-LargeRef` and `objsnowHD-LargeRef`. Both blocks contained a `BSDistantObjectLargeRefExtraData` child block, name `DOLRED`, containing only a single bool (`true`) identifying the blocks as large ref LOD. Note that the `BSDistantObjectLargeRefExtraData` block type didn't exist in LE.


