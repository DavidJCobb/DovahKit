
# Collision layers

Collision layer forms (signature `COLL`, internal class name `BGSCollisionLayer`) serve to let Bethesda define collision layers and their relationships in the Creation Kit. You'll notice that I said "Bethesda" and "the Creation Kit," not "you" or "xEdit and DovahKit." Due to how these forms are designed, it's not safe for us to create new ones, and some edits to vanilla forms would be highly problematic as well.

Under the hood, collision layers have a unique ID (serialized as `COLL/BNAM`). When creating NIFs, you can set a physics shape's Collision Layer to a value matching these UIDs in order to associate the shape with that layer.


## Safe editing

Basically, you can change anything except `BNAM`. You can change the list of layers that collide with any given other layer (serialized as `COLL/CNAM`). You can change a layer's name, description, and debug color. You can change the "Trigger Volume" and "Sensor" flags, though doing that will likely break content authored under the expectation of particular layers having those flags.

## UIDs

The vanilla collision layers have UIDs that are hardcoded into the game engine. For example, when the engine wants to do something with `L_BIPED`, it won't look up the `[COLL]L_BIPED` and use that form's `BNAM` value; it'll just use value `8`. Several layers have hardcoded relationships or behaviors, so changing a vanilla layer's `BNAM` value is ill-advised; it'll cause the `COLL` form (and its subrecords, e.g. `COLL/CNAM`) to become divorced from what the engine's hardcoded behaviors expect.

A `COLL` form's settings (`CNAM`, etc.) are stored and indexed[^storage] by UID. Thus, if two `COLL` forms have the same UID, one of them[^overwrite-priority] will overwrite the other's settings.

[^storage]: They're stored on `bhkCollisionFilter`, a singleton.

[^overwrite-priority]: After the game has loaded all forms from all data files, it iterates over an array of all loaded collision layer forms. I don't know the precise order in which forms are placed in these arrays, but I suspect that the form whose *base record*, in specific, was loaded last would win a UID conflict. This would differ from the Rule of One's priority.

Collision layer settings are stored as a struct-of-arrays, with each array having 64 entries, and the `CNAM` mappings are stored as 64-bit masks; thus UIDs cannot functionally be higher than 63. Additionally, UIDs cannot safely be higher than 127 per the next section.

The narrow ID space available means that there is an extremely high risk of conflicts if multiple mods create their own collision layers. This is especially problematic with the Creation Kit, as it seems to set IDs sequentially (i.e. a file with Skyrim.esm as its only master will always have its first new `COLL` use UID 55, coming after 54, the highest vanilla UID) such that mods created with the CK are *guaranteed* to conflict if they create new `COLL` forms.

### Havok collision filter infos

When programming with the Havok physics engine, you use four-byte integers as "filter infos" to determine whether any two shapes are allowed to register contact/collision with one another. The precise meaning of these integers is user-defined. Bethesda packs `COLL/BNAM` into the lowest 7 bits, which constrains the max safe value of `BNAM` to 127. Bits [8, 12] are a body part index (from shapes defined in NIF files). Bit 14 means a shape will only register a collision with `L_CUSTOMPICK2`, and bit 15 means the shape won't register collisions at all in most (all?) scenarios. The two most-significant bytes are a system group; it seems that each active ragdoll gets its own system group, and this is used to prevent body parts from colliding with one another if they're directly connected.

Thus, `COLL/BNAM` or `REFR/XTRI` values above 127 would overlap bits unrelated to the collision layer index, causing physics instability. I can personally attest to a large trigger volume with `REFR/XTRI` set to `0x00C0BB01` causing the game speed to slow to something measurable in "seconds per frame." When the game creates the Havok collision volume for a trigger activator, it sets the volume's "broad phase handle collision filter" to `XTRI | 0x000B` i.e. system group 11; nothing is done to constrain the original `XTRI` value.

## Hardcoded layers with hardcoded changes

The game will dynamically overwrite the layer-to-layer collision masks (i.e. the place where `COLL/CNAM` ends up) for layer UIDs 43 (`L_CUSTOMPICK1`), 44 (`L_CUSTOMPICK2`), and 45 (`L_SPELLEXPLOSION`) during gameplay. I haven't reverse-engineered the exact configurations or reasons, but I would presume that this is done whenever the game needs to check or raycast for shapes in specific layers. Thus mods should not depend on these layers/UIDs having any particular configuration (i.e. it would be foolhardy to use these UIDs in `REFR/XTRI`).
