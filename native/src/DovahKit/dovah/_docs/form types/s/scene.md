
# Scene

## Record data

### Evolution

We can determine a few things about how the record format evolved during development by looking at leftover data in Skyrim.esm.

* Prior to record version v31, scenes possessed multiple legacy scripts, with the Creation Kit attaching empty scripts by default. As of v31, the Creation Kit no longer added legacy scripts to scenes by default, but it didn't actively remove any that were already present. At some point no earlier than v32 and no later than v34, the Creation Kit actively removed legacy scripts.

  * Each phase that has legacy scripts has two such scripts, separated from each other with a `NEXT` subrecord. (They are *not* separated from the preceding condition lists, even though those lists are separated from each other.) It's not clear whether phases were able to have an arbitrary number of scripts, or whether they always had two. The Creation Kit no longer has code to load these, and I don't feel comfortable guessing at how it handled `NEXT`, so DovahKit makes no attempt to load this data.

  * Only the following scenes possess non-empty legacy scripts:

    * [SCEN:0001B113]DGScene04: Phase 0 script 0 has a single instruction: `set TestSceneCount to TestSceneCount + 1`.

  * The scene as a whole also had two such legacy scripts, separated with `NEXT`. It's possible that the script pairs (for both the scene as a whole and for its phases) are "on begin" and "on end." As with per-phase scripts, I can't be certain of how many scripts should be present nor of their separator logic, so DovahKit simply doesn't load them at all.
  
* `SCEN/Dialogue Action/DEMO+DEVA` were introduced in v25.

* `SCEN/Actor/LNAM` was introduced in v30.
* `SCEN/Actor/DNAM` was introduced no earlier than v24 and no later than v25.
* `SCEN/VNAM` was introduced in v31, but wasn't guaranteed to be present on SCEN records until a version no earlier than v32 and no later than v34.
