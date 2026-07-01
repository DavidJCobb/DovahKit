# `BGSBodyPartData`

## Notes

* The game and Creation Kit store body parts within a `BGSBodyPartData` form differently.

  * The game stores the body parts as a `std::array<BodyPart, 6>`. The index in the array corresponds to the limb that the body part identifies itself as belonging to, such that e.g. the last-loaded "FlyGrab" part is the only one retained. (If a freshly-loaded body part identifies itself as belonging to an invalid limb, it is discarded.)

  * The Creation Kit stores them as a `BSTList<BodyPart>` i.e. a linked list of arbitrary length. This allows the CK to fully validate all parts in the list.

### Edge-cases

* The CK detects and warns on the following cases:

  * The form specifies no skeleton model.
  * The form specifies a skeleton model that doesn't exist or otherwise can't load.
  * Two body parts list the same node name as their main node.
  * Two body parts apply to the same limb.
  * A body part fails to load (i.e. `BGSBodyPartData::BodyPart::Load` returns `false`).
  * A body part specifies an invalid limb. (In this case, the CK skips loading it entirely.)
  * A body part specifies no main node name.
  * A body part specifies a main node name that isn't present in the skeleton NIF.

  * After loading a body part, the currently-open subrecord belongs to the next body part (i.e. `BPNN`/`BPTN`/`PNAM`) or to content after the body parts (i.e. `RAGA`). This is not a fatal error; both the CK and the game "attempt recovery" (per the CK's wording) by just... continuing to read from the current subrecord, instead of inadvertently skipping it by opening the next one; and that should work just fine. (Hell, when I saw this behavior in the game's loader, which doesn't emit warnings, I mistook it for intentional design and not an error correction procedure.)