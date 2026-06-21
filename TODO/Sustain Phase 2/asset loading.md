
# Asset loading

Asset loading is asynchronous, but not multi-threaded. When you request a NIF, you're given a `rendered_nif` object, but the process of loading that NIF's data will be carried out at some point in the future. It's asynchronous, but it's not concurrent with the operation of the rest of the program: the load is carried out on the main thread during the process of drawing a frame. **Assets aren't loaded in the background.**

Assets are loaded concurrently *with each other*: textures are loaded side-by-side; NIFs are loaded side-by-side (partially); et cetera.

The precise process is:

* **Load textures:** Load any queued textures.
  * Use workers to load the textures from the filesystem/BSAs onto the CPU as `rendered_texture`s, and to queue a CPU-to-GPU copy.
* **Load NIFs:** Load any queued NIFs.
  * Use workers to load the NIFs from the filesystem/BSAs onto the CPU.
  * For each `rendered_nif`:
    * Reserve `rendered_mesh` slots for all geometry in the NIF.
    * If the to-be-loaded form has a `TESModelTextureSwap`, apply any configured texture-swaps to the CPU-side NIF blocks.
      * For each block using a texture swap, either load the TextureSet form data into memory and modify `BSShaderTextureSet` data in place, or (if the texture swap is to `NullTextureSet`) cull the block entirely.
    * **Load meshes:** Create "active, background-loading" `rendered_mesh`es for non-empty geometry blocks in the NIF, and process their textures: associate them with any already-loaded textures, and queue any new textures to load.
  * **Load textures:** Check if any new textures have been queued to load. If so, use workers to load them now.
  * For each of the `rendered_mesh`es queued to load above, copy their data (VIB, flags, etc.) from the source NIF block to the `rendered_mesh`.
  * Mark all of these NIFs as no longer being in background use (i.e. no longer queued to load, loading, or generating meshes).
* Mark swap chain frames-in-flight to perform CPU-to-GPU copies for changed scene entity lists.

Possible optimizations:

* Move processing of texture swaps off of the main thread and into the worker threads for NIFs.
  * Required changes
    * When queueing a NIF to load, copy the base form's texture-swap data (if any) immediately, so the load process doesn't have to worry about managing a loaded-form smart pointer for the base form. (Form data can only safely be (un)loaded on the main thread.)
    * When queueing a NIF to load, load the TextureSets in its swap data immediately and store the texture paths for each swap. That way, we don't need to (un)load the TextureSet form data while processing the freshly-loaded NIFs.
  * Questions
    * What happens if the base form for a NIF (or one of the TextureSets used in its swaps) changes between a NIF being queued to load, and us fully loading the NIF? How do we detect that form data which might influence the NIF has changed, and re-update it?
      * Worldedit was planned to listen for this and handle it, but that was never implemented. I guess we could just make Worldedit responsible for that when we do this refactor.
      * Really, the renderer shouldn't know or care about base forms or TextureSet forms at all. It should just take the file path of a NIF, and a list of any applicable swaps, and handle that. Worldedit should be responsible for extracting those swaps, and it should be responsible for detecting changes to any "influencing forms" for any particular ref. (To avoid excess redundancy, it'd want to store a map of base forms to relevant TextureSets, rather than having every ref store a list of TextureSets. It might also want to store a set of all TextureSets, for fast checking of whether a modified TextureSet is render-relevant.)
* Load textures after NIF files, rather than before *and* after.
  * This will avoid potential edge-cases where we have, like, one texture already queued to load, so we spin up four texture workers and have three of them idle; and then we load a NIF that wants three more textures, so we spin up four texture workers and have one if them idle.
  * Specifically, we'd want this to be the high-level process: load NIFs, if any; load meshes, if any[, potentially queueing texture loads]; load textures, if any; notify frames-in-flight about scene entity additions.

