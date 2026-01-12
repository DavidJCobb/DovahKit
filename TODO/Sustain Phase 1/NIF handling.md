
# NIF handling

Our NIF loader is pretty decent. Unfortunately, however, it's only capable of loading NIFs; we can't save NIFs back out. This prevents us from doing two things:

* **Updating `MATO/DNAM`.** Material Objects rely on `NiProperty` data. Although Material Objects take a NIF file as input, and they *do* store that file in a `TESModel` component, they don't treat that file as the canonical source of their property data.
  
  Rather, whenever you select a NIF in the Creation Kit, they assume that NIF is a `NiNode` and check if its first child is a `NiGeometry`; if so, they pull out the geometry's `NiProperty` objects and retain them in memory. At save time, they serialize the properties into `MATO/DNAM`: each DNAM subrecord is a binary stream containing a NIF file, whose contents are the `NiProperty` and any child blocks (i.e. `Ref`s in NifSkope parlance). In-game and in the CK, `DNAM` is considered the canonical source of property data.

* **Baking FaceGen heads for `NPC_`.**

* **BGSStaticCollection forms.** Early reverse-engineering suggests that this scrapped form type worked by automatically combining the NIFs of all constituent Statics into a single NIF, and saving that NIF out as a file for the Static Collection form to use.

During Sustain Phase 1, we need to redesign our NIF handling so that we can both load *and save* NIFs, in both LE and SSE formats.

## Concerns for saving data

* References to NIF blocks are sensitive to the order in which blocks are loaded. To save NIF files out (not merely round-tripping a loaded file, but being able to save individual blocks and their dependencies, for the sake of `MATO/DNAM`), we need an analogue to NifSkope's "Reorder Blocks" function, and we need the ability to gather all to-be-saved blocks in the first place.
  * In other words, blocks need a virtual member function which allows us to run a `std::function` on all outbound references to other blocks, so that given some block that we definitely want to save, we can accumulate any additional to-be-saved blocks and sort them by how they're referenced.
    * Well, actually, Ref always points down the hierarchy (from lower-numbered blocks to higher-numbered blocks), while Ptr always points up the hierarchy (from higher-numbered blocks to lower-numbered ones). So, NIF blocks would have to make that distinction, and we'd need a member function which can accumulate Refs and Ptrs separately.
      * So... `ref_to_prior<T>` for Ptr and `ref_to_after<T>` for Ref?
      * If Ref has ownership semantics, then `owned_ref<T>` (Ref) and `unowned_ref<T>` (Ptr) would be better names.
* We're likely going to want or need to be able to convert between LE and SSE NIF data. In some cases, this means changing entire block types and coalescing multiple blocks together, e.g. for a conversion between `NiTriShape`+`NiTriShapeData` and `BSTriShape`.
  * Conversion should be a wholly separate operation from saving. However, it should be possible to clone the to-be-saved blocks into their own block tree/collection, convert just that tree, and then save it.
* Perhaps, then, we should have a general notion of a "block tree," which stores a flat list of all blocks, and a single root `NiObject` if one exists.