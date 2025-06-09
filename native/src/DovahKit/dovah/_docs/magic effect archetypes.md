
## Darkness

Implemented as `DarknessEffect`, a subclass of `ValueModifierEffect`. It has the additional effect of recursively walking the target actor's `NiNode` tree to modify a field on all `BSLightingShaderProperty` objects it finds. This field is a special value dedicated to `DarknessEffect` specifically.

The field is set to the current value of the effect's target AV, divided by 100 and clamped to the range [0, 1].

## Disguise

Implemented as `DisguiseEffect`. This effect is hardcoded to disguise the player as the target actor, if there is one: the `PlayerCharacter` structure tracks a refhandle (called `assumedIdentity` in CommonLibSSE), and some combat-related game internals are coded to check that handle.

* `IsInFaction` checks on the player will check if they have an assumed identity, and if so, the checks are run against that identity rather than against the player.
* The effect creates a `DisguiseEffect::DetectionChecker`, a subclass of `DetectionCollector` (itself a subclass of `DetectionListener`). Further RE is required to determine this object's precise behavior.

On effect start, the player's assumed identity is set to the target actor, which has the side effect of forcibly stopping combat between the player and the target; the effect then forces the target to update their combat state. On effect finish, if the player's assumed identity is still the target actor, then the assumed identity is wholly cleared.

The effect has an internal state value. Whenever the effect re-processes conditions, it checks if this state value is anything other than "normal" (the other possibilities are "initializing," "waiting," and "fail"); if so, the effect forces the game to act as though the conditions were not met, thus deactivating the effect. Presumably, this is used in combination with the `DetectionChecker` to cause the disguise to fail under certain circumstances.

## Open

Implemented as `OpenEffect`. On effect start, this gets the target ref and checks if said ref is an actor; if so, it aborts immediately. Otherwise, it checks if the target ref has any `ExtraLock` data; if so, it treats the effect magnitude as a Lockpicking skill level, and converts both that value and the lock's skill level to an enum (Novice, Apprentice, et cetera). If, within this enum, the lock is weaker than the spell, then the ref is unlocked instantly and with no side effects (i.e. this doesn't count as a crime or anything). Otherwise, nothing happens.

Of course, since Magic Effects can no longer be applied to arbitrary refs, none of this behavior does anything anymore.

## Script

Implemented as `ScriptEffect`, which can store a pointer to a legacy `Script` and its `ScriptLocals`. If the effect has a target and a script pointer, its start, update, and finish handlers invoke that script in different ways. Reverse-engineering this any further is low-priority, since legacy script forms can't be created in the CK or in xEdit.

## Spawn Hazard

Implemented as `SpawnHazardEffect`. Creates a Hazard (the base form is `BGSHazard`; refs are `Hazard`). When the effect finishes, it tells the hazard to die; the hazard sets its lifetime to `fHazardMaxWaitTime` and then adjusts its internal state in other ways (possibly to trigger an animation for its destruction).

## Spawn Scripted Ref

Implemented as `ScriptedRefEffect`, a subclass of `ScriptEffect`. This doesn't override any virtual member functions and so has identical hardcoded behavior to the Script archetype.

## Werewolf Feed

Implemented as `WerewolfFeedEffect`. Sets the "has been eaten" flag on the target, and then attempts to dispel itself.