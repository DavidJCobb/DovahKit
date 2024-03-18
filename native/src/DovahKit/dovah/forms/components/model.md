
## Add-ons

One of the things that `TESModel` can pre-cache is a list of add-on node IDs. But what the hell do an of those words mean?

Bethesda built a system called **add-ons**, which basically allows one NIF to include the contents of another NIF by reference. You can't specify arbitrary NIF files; rather, the game data defines a library of reusable NIFs.

* In your ESP file, you can define an Add-on form (`ADDN` i.e. `form_type::addon_node`) which consists of a 3D model and a unique numeric ID.
* In any NIF file, you can create a BSValueNode. This is, functionally, just a local transform (i.e. offset from its parent node) and a value. In practice, however, the "value" specifies one of those unique numeric IDs. Skyrim will look up the appropriate `ADDN`, load its NIF, and copy the contents of that NIF to where the BSValueNode is.

Here are a few examples:

* `Effects\CWFXCatapultProjectile.nif` uses several add-on nodes with IDs 5, 6, 71, and 74.
  * The ID 5 is defined by `[ADDN:0001CBB2]MPSFireBallSparkTrail`.
  * The ID 6 is defined by `[ADDN:0001CBB1]MPSFireBallSmokeTrail`.
  * The ID 71 is defined by `[ADDN:000E1532]MPSDragonFireWave`.
  * The ID 74 is defined by `[ADDN:000EE9ED]MPSFireBallSmokeTrailBig`.

  Each of these Add-on Node forms specifies a NIF file; so, a Civil War catapult projectile can include the 3D effects for sparks, smoke, flames, and smoke trails entirely by reference.

* `Effects\AshPile01.nif` uses a single add-on node with ID 61. That ID is defined by `[ADDN:00E7556]MPSAshPile01`, which seems to be purpose-built for this particular model. At this time I don't have the means to easily search and see if anything else uses it.

Which brings us back to the `TESModel` data, in the `MODT` and similar subrecords. This data will list all add-on IDs used by the NIF, so that when the game wants to load the model, it can also go and load those add-on node definitions' NIFs with it.