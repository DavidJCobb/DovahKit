
## High-level feature overview

### Things the renderer supports

* Alpha-blended and alpha-tested geometry (via weighted blended order-independent transparency)
* Cell borders
* Cell landscape
* Cell lighting params (ambient, fog, etc.)
* Edit gizmos
* Landscape
  * Textures
    * Diffuse
    * Normal
* Light emitters [partial]
  * Omni
  * Omni, Shadow
  * Hemi
  * Spot
* Meshes [partial] (basically any `BSTriShape`/`NiTriShape` stuff)
  * Alpha blending and testing
  * State
    * "Culled by application" flag
  * Textures
    * Diffuse
    * Normal
  * Some shader parameters
    * "Receive shadows"
  * Specular
    * Color
    * Exponent
    * Strength
* Object bounds (for selected objects)
* Shadows


### Things the renderer does not support

Some of these will first require support within our NIF loader. Some (e.g. door teleport markers) will require support in Worldedit.

* Actors
  * Animations
  * Morphs (i.e. `.tri` files)
  * Skinning/armatures (i.e. for clothing)
  * Weight interpolation
* Arbitrary text[^render-window-ui]
* Door teleport markers
* EffectShaders and the underlying NIF effect-shader properties
* Grass
* Light emitters [partial]
  * Animations (flicker, pulse)
* LOD levels (some meshes can define geometry to be hidden at different distances)
* LOD meshes (i.e. cell LODs baked with the Creation Kit)
* Meshes [partial]
  * Unique shader types (hair, etc.)
* Navmeshes
  * Cover
  * Triangles
* Particles
* Primitives (i.e. geometry from `ExtraPrimitive`)
  * Box
  * Plane
  * Sphere
* Sky
* Tree physics
* Water
  * Cell water planes
  * Specific water shaders for water-activators (the geometry renders as solid white right now)
* Weather
  * Time-of-day support

[^render-window-ui]: One of the things I eventually want is to be able to show UI, including menus, entirely within the Render Window. Think of the menus for Halo's Forge mode and things like that. This will require, at minimum, the ability to render arbitrary text (ideally using a palette of glyph SDFs).


### Things I'd need to double-check

* Animations
  * Havok stuff definitely isn't supported, and I doubt we do much for Gamebryo animations, but we have an `anim_state` pointer for some rendered-mesh objects, which suggests to me that we at least have some simple hooks for simple animations. This *may* have only been used in the earliest stages of debugging the renderer.
* Model-space normals (needed for actor bodies and faces)
* `NiLines` and friends

### Nice-to-haves

* Render a view of cell visibility data (i.e. which cells the CK thinks should be culled when the camera is in which other cells)
* Render a view of the max height data
