
# Handling form changes

Right now, there are some major defects with how Worldedit handles and applies changes to form data:

* Worldedit doesn't handle form creations.
* Worldedit doesn't fully handle form deletions.
  * Code to handle deletion of non-ref cell children (i.e. landscapes, navmeshes) is unimplemented.
  * Code to handle deletion of base forms is unimplemented.
* Worldedit doesn't handle form modifications made by outside systems.
  * Code to handle alterations to loaded cells is unimplemented.
  * Code to handle alterations to refs is unimplemented.
  * Code to handle alterations to cell children (i.e. landscapes, navmeshes) is unimplemented.
  * Code to handle alterations to base forms is unimplemented.

When you move refs using Worldedit, those changes are immediately reflected in the Render Window. However, DovahKit never dispatches important notifications or changes related to modifying the ref, so nothing else in the program reacts:

* `DovahKitCore::formModificationImminent` and `DovahKitCore::formModified` aren't emitted, so it wouldn't be possible for other parts of the program (e.g. a REFR dialog) to react to the change even if they were programmed to.
* `some_form_stub->set_edited(true)` is never called, so DovahKit as a whole may not even know that the ref has been modified and needs to be saved in the active file!

Similarly, moving a ref into a non-existent exterior cell can trigger creation of that cell. Worldedit loads the cell instantly and manually, rather than listening for e.g. `DovahKitCore::formCreated`.


## Outside changes that Worldedit must detect and handle

* Form creation
  * Exterior cells created within the loaded area must be loaded.
  * Refs that are created within the loaded area must be loaded.
* Form deletion
  * Handle:
    * Landscapes
    * Navmeshes
  * Ignore base form deletions. Any refs using those bases should dispatch `formModified` before the base form dispatches `formDeletionImminent`, and we only really care[^base-form-deletions-and-opals] about the refs that would be impacted, so listening for modifications to refs should be sufficient.
* Form modification
  * Changes to a loaded cell must be applied.
    * Cell lighting
  * Changes to a loaded cell-child must be applied. (How do we handle this if Worldedit itself is what triggered the change?)
    * Landscape
      * LandTextures used
      * Vertex heights
      * Vertex paint values for colors
      * Vertex paint values for textures
    * Navmesh
    * Ref
      * Base form
      * Extra-data (to draw lines, indicators, etc.)
        * Action (flags indicate whether a Door ref is open by default)
        * Alpha cutoff
        * Attach ref
        * Count (affects rendering of Ammo refs)
        * Emittance source (affects rendering of Light refs, at a minimum)
        * Enable state parentage
        * Leveled creature modifier (affects color of LeveledActor refs)
        * Light
        * Linked ref
        * Linked ref color
        * Multibound bounds
        * Multibound ref
        * Navmesh door portal (to indicate linked/unlinked teleport markers)
        * Occlusion plane
        * Occlusion plane ref data
        * Portal
        * Portal origin and destination
        * Primitive
        * Radius
        * Room ref data (roombound lighting and imagespace)
        * Scale (see below)
        * Spawn container (CK doesn't draw this but we probably should)
        * Teleport
        * Time left (should affect rendering of Light refs, if zero?)
      * Position/rotation/scale
  * Changes to a base form must be applied, if that base form is used by any loaded refs.
    * "Is Marker" flag (once we implement CK-style Show/Hide filters)
    * Model path
    * Model texture swaps
  * Changes to a TextureSet must be applied, if that TextureSet is used by the LandTextures painted on any loaded landscapes, or if the TextureSet is used for a texture swap by the base forms of any loaded refs.

[^base-form-deletions-and-opals]: *Well...* This may not be *entirely* true. If we implement the ability to load and navigate Object Palettes, then we'll care about base forms, particularly if we have gamepad-accessible menus within the Render Window (which is a far-future goal). But that's far enough down the line that we can completely ignore it for now. Whatever system we implement for that should be self-contained and should be responsible for catching base form deletions itself.

