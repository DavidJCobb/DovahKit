### Data types

#### Package Data types
Packdata identify their value types via a string in `PACK/ANAM`. The values are in `CNAM`.

* Bool
  * Value is CNAM storing a bool.
* Float
  * Value is CNAM storing a float.
* Int
  * Value is CNAM storing an integer.
* Location
  * Value is PLDT.
* LocPt *[found via RE]*
  * ?
* LocCell *[found via RE]*
  * A single interior cell?
* ObjectList
  * Value is CNAM?
* SingleRef
  * Value is PTDA. (Are there other possibilities or constraints? For example, is PTDA here constrained to package location types that produce a REFR, including reference aliases?)
* TargetSelector
  * Value is PTDA.
* Topic
  * Value is PDTO[] or TPIC.

#### Procedure branch types
Procedure branches identify their types via a string in `PACK/ANAM`.

* Procedure
  * Indicates a leaf node: an actual AI procedure.
* Sequence
* Stacked
* Simultaneous
* Random

#### Procedure types
Procedure-type branches identify the procedure via a string in `PACK/PNAM`. Procedure names not documented on the wiki are italicized.

* Acquire
* Activate
* *DialogueActivate*
* *Dialogue*
* *Done*
* Eat
* Escort
* Find
* Flee
* FlightGrab
* Follow
* Guard
* HoldPosition
* Hover
* KeepAnEyeOn
* LockUnlock
* Orbit
* Patrol
* Sandbox
* Say
* Shout
* SitSleep
* Travel
* UseIdleMarker
* UseMagic
* UseWeapon
* Wait
* Wander
