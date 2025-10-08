
# Packages

In Skyrim, an AI Package is, broadly, a tree of hardcoded AI Procedures given various inputs (called "package data") and conditions in order to direct an NPC's non-combat behavior. However, in order to understand how Package forms are structured in memory and in serialized data, we must understand what Package forms were in pre-Skyrim games.

What we call Procedures in Skyrim were Package Types in previous games. In Skyrim, you might have a package which contains a Flee procedure; in previous games, you'd have a Flee package. In previous games, the term "package data" referred to the type-specific data for a Package: subclasses of `TESPackageData`, such as `TESAmbushPackageData`.

The key thing is that Skyrim's system wasn't a total replacement; rather, the old system incrementally evolved into the new one. Modern packages are just a new package type, `TESCustomPackageData`, and the Creation Kit UI offers no means to create packages using the legacy types. The objects that are called "package data" now are subclasses of `BSGPackageData`, e.g. `BGSPackageDataFloat`.

Unfortunately, because Skyrim's Package implementation is a complete redesign implemented within the original design, we can't simply ignore the legacy concepts and terminology; the Creation Kit is in fact capable of loading some legacy package types.

So let's define some terms in order to deal with the ambiguity in Bethesda's own terminology.

| DovahKit term | Bethesda term | In-game base class | Legacy? | Description |
| :- | :- | :- | :- | :- |
| Package | Package | `TESPackage` | No | An AI package form. |
| Typed package info | Package data | `TESPackageData` | Yes | Type-specific package content. |
| Custom Package | Custom Package | `TESCustomPackageData` | Uh? | Modern package data, as wrapped in a legacy-style package type. |
| Package data | Package Data | `BGSPackageData` | No | The "variables" within a Custom Package. |

## Loading

Each (still-recognized) package type has a header subrecord, which signals the presence of typed package info and possibly the presence of additional subrecords dedicated to typed package info. For example, custom packages use `PKCU` (PacKage CUstom) as their header, while ambush packages use `PKAM` (PacKage AMbush) as their hader.

When the Package form loader encounters such a header subrecord, it deletes any `TESPackageData*` that the package may already have, and then instantiates a new `TESPackageData*` of the appropriate subclass. Legacy package types stop there, with any additional subrecords handled by the form loader; however, `TESCustomPackageData`'s constructor also consumes additional subrecords in its own subrecord-reading loop.

Complicating things is the fact that some data is specific to legacy package types, but is stored on the `TESPackage` form directly (and so wouldn't be cleared if, say, a Package first claimed to be Flee and then Custom). Specifically, legacy packages can have a *primary location* and a *primary target* stored on the Package form itself, and some specific legacy package types also support a *secondary location* and a *secondary target* stored within typed package info.

~~The way to handle this, then, is as follows:~~ **EDIT: there are technical issues with this approach; gotta do something different**

* Polymorphic classes for each typed-package-info type, including Custom.

* These polymorphic classes can store package-targets and package-locations as appropriate, with names befitting their purpose within the package type (e.g. Ambush info would store a Trigger Location and an Ambush Location).

* These polymorphic classes will have virtual functions which return pointers to their primary and secondary locations and targets (or `nullptr` if they have no such location). That way, when you *know* you're working with Ambush package info, you can talk in terms of "the trigger location" whereas when you don't specifically know what package type the info is for, you can (via the acessors) talk in terms of "the primary location, if any."

* The Package form loader has to retain the primary location and primary target as locals during load. When we've finished reading all subrecords, we can check if we have any typed package info and if so, feed the primary location and target into said info via its polymorphic accessors.


## Notes

* The Creation Kit allows you to reorder a package's packdata, if that package does not use a template. The ordering is based on the order the values appear within the record; the order of the declarations has no effect.


# Research

## Form serialized data

```
PACK
   EDID
   VMAD
   PKDT // Base data
      uint32_t                        flags;
      package_type                    type;
      package_interrupt_override_type interrupt;
      uint8_t                         preferred_speed; // enum
      uint8_t                         pad07;
      uint16_t                        interrupt_flags;
      uint16_t                        pad0A;
   PSDT // Schedule
      int8_t  month;
      int8_t  weekday;
      uint8_t date;
      int8_t  hour;
      int8_t  minute;
      uint8_t pad05[3];
      int32_t duration; // in minutes
   IDLF+IDLC+IDLT+IDLA+IDLB
      BGSIdleCollection idles;
   CNAM // Combat Style
   QNAM // Owning Quest
   TESCustomPackageData
      PKCU // triggers a separate subrecord-reading loop
         uint32_t   data_input_count;
         form_stub* package_template;
         uint32_t   version_counter; // CK warns if it'd overflow a two-byte int
      BGSPackageDataList
         //
         // First, the list of values we're supplying as package parameters; then, 
         // the list of identifiers (unique IDs and names) for those values. These 
         // are basically paired arrays, I guess.
         //
         Package Data[data_input_count]
            //
            // If the first subrecord is not ANAM, then the loader warns. Of course, 
            // if a package data begins with non-ANAM, is of an unknown type, or has 
            // an unknown subrecord, then the loader will inevitably fail on it and 
            // everything after it.
            //
            ANAM // Typename
            BGSNamedPackageData<...>
               std::optional<BNAM> // legacy equivalent to UNAM/BNAM?
                  std::string unk00; // MAX_PATH
               std::optional<PNAM> // legacy equivalent to UNAM/PNAM?
                  uint32_t flag_value; // if non-zero, we set flag 0. "public" flag??
            (CNAM|BGSPackageDataLocation|BGSPackageDataTopic|BGSPackageDataRef|BGSPackageDataTargetSelector) // Value
               CNAM
                  union { bool; int32_t; float; }
               BGSPackageDataLocation
                  PLDT // "Package Location DaTa"
                     // ...
               BGSPackageDataTopic
                  TPIC // "ToPIC" (deprecated; we only save PDTO back out)
                     form_stub* topic;
                  PDTO // "Package Data TOpic"
                     // ...
               BGSPackageDataRef
                  PTDT // "Package Target DaTa" // deprecated; prefer PTDA
                     // ...
                  PTDA // "Package Target DAta"
                     // ...
              BGSPackageDataTargetSelector
                  // effectively identical to the contents of BGSPackageDataRef
         Unique ID[data_input_count]
            __subrecord[data_input_count]
               UNAM
                  uint8_t index; // 0xFF = none
               *
                  If the N-th subrecord is anything other than UNAM, then the 
                  loader acts as though it were UNAM with value N. Additionally, 
                  the default value used if XNAM is absent will be changed to 
                  (N + 1) truncated to a byte.
               //
               // The loader warns on value 0xFF with the text "Hey, how'd this 
               // get here?"
         XNAM
            uint8_t next_unique_id;
      BGSProcedureTreeBranch[] // Procedure Tree Node
         ANAM // branch typename
         CITC+CTDA[]
         PRCB // "PRoCedure Branch"
            uint32_t branch_count;
            uint32_t branch_flags;
            //
            // Given a branch count of X, the next X nodes we load will be 
            // direct children of this node... UNLESS one of THOSE nodes has 
            // Y children, in which case our child list is "interrupted" by 
            // its own.
            //
         BGSProcedureBase
            PNAM // procedure typename
            FNAM
               uint32_t procedure_flags;
            PKC2[] // "PacKage Custom 2"?
               uint8_t unique_id;
            std::optional<Flags Override>
               PFO2 // "Package Flag Overrides 2" // BGSProcedureBase instance's flag overrides
                  uint32_t set_general_flags;
                  uint32_t clear_general_flags;
                  uint16_t set_interrupt_flags;
                  uint16_t clear_interrupt_flags;
                  uint8_t  preferred_speed_override;
                  uint8_t  pad0D[3];
            std::optional<Legacy Flags Override>
               PFOR // "Package Flag OverRides" // same as PFO2 but without preferred speed
                  uint32_t set_general_flags;
                  uint32_t clear_general_flags;
                  uint16_t set_interrupt_flags;
                  uint16_t clear_interrupt_flags;
                  uint8_t  preferred_speed_override;
                  // preferred speed is set to 2 i.e. Run
      Package Data Name Map Entry[]
         UNAM
            uint8_t unique_id
         BNAM
            std::string name
         PNAM // Public
            uint32_t flags
      OnBegin Event Action
         POBA // empty marker
         INAM
            form_stub* idle;
         SCHR+SCTX+QNAM+TNAM // legacy script
      OnEnd Event Action
         // ditto
      OnChange Event Action
         // ditto
End.
```

### Other package types
NOTE: Loading any package type deletes the type-specific data for any previously-loaded package type.

Legacy types, per the loader:

```
PKE2 // TESEscortPackageData
   uint32_t distance
PKDD // TESDialoguePackageData
   float      fov
   form_stub* topic
   bool       no_headtracking
   bool       dont_control_target_movement
   uint16_t   pad0A
   uint32_t   dialogue_type
      // 0 == conversation
      // 1 == say to
   uint8_t    unk10
   uint8_t    unk11
   uint8_t    unk12
   uint8_t    unk13
   uint32_t   unk14
PKFD // TESFollowPackageData
   float start_location_trigger_radius
PKW3 // TESUseWeaponPackageData
   uint32_t flags
            // 1 <<  0: Always Hit
            // 1 <<  8: Do No Damage
            // 1 << 16: Crouch To Reload
            // 1 << 24: Hold Fire When Blocked
   uint8_t  fire_rate
      // 0 == auto_fire
      // 1 == volley_fire
   uint8_t  fire_count
      // 0 == number_of_bursts
      // 1 == repeat_fire
   uint16_t number_of_bursts
   uint16_t shots_per_volley.min
   uint16_t shots_per_volley.max
   float    pause_between_volleys.min
   float    pause_between_volleys.max
PKCU // TESCustomPackageData
   a ton of stuff...
PKPT // TESPatrolPackageData
   bool    repeatable
   uint8_t unk01
```

Legacy data (per the loader):
* **PTD2:** type 0x10
* **PLD2:** types 0x0F, 0x10
* **PT2A:** type 0x10

Legacy data (per the writer):
* Secondary location: 1, 2, 3, 8, 9, 15, 16

Legacy types, per save code:
```
TESAmbushPackageData
   PKAM
      <empty>
   PLD2 // secondary location
      <PackageLocation>
TESDialoguePackageData
   PKDD
      <as above>
   PLD2 // secondary location
      <PackageLocation>
TESEatPackageData
   PKED
      <empty>
   PLD2 // secondary location
      <PackageLocation>
TESEscortPackageData
   PKE2
      <as above>
   PLD2 // secondary location
      <PackageLocation>
TESFollowPackageData
   PKFD
      <as above>
   PLD2 // secondary location
      <PackageLocation>
TESPatrolPackageData
   PKPT
      <as above>
TESUseItemPackageData
   PUID
      <empty>
   PLD2 // secondary location
      <PackageLocation>
TESUseWeaponPackageData
   PKW3
      <as above>
   PT2A // secondary target
      <PackageLocation>
   PLD2 // secondary location
      <PackageLocation>
```

### Package whole-data types

| # | Name | Header | Data class | Has Target&nbsp;1 | Has Target&nbsp;2 | Has Location&nbsp;1 | Has Location&nbsp;2 | Notes |
| -: | :- | :-: | :- | :-: | :-: | :-: | :-: | :- |
| 0 | Find | | - | Yes | No | Optional | No | Location 1 is a Search Location. |
| 1 | Follow | `PKFD` | `TESFollowPackageData` | Yes | No | Optional | **Optional** | Location 1 is the End Location. Location 2 is the Start Location. Target 1 is the ref to follow. |
| 2 | Escort | `PKE2` | `TESEscortPackageData` | Yes | No | Yes | **Optional** | Location 1 is the Destination. Location 2 is the Search Location, if Allow Search is enabled. Target 1 is the Escort Target. |
| 3 | Eat | `PKED`? | `TESEatPackageData` | Yes | No | Yes | **Optional** | Location 1 is the Eat Location. Location 2 is the Search Location, if Allow Search is enabled. Target 1 has its type forced to 2. |
| 4 | Sleep | | - | No | No | Yes | No |
| 5 | Wander | | - | No | No | Yes | No |
| 6 | Travel | | - | No | No | Yes | No |
| 7 | Accompany | | - | Yes | No | No | No | Target 1 has its type forced to 0. |
| 8 | Use Item At | `PUID` | `TESUseItemPackageData` | Yes | No | Yes | **Yes** | Location 1 is the Location. Location 2 is the Search Location, if Allow Search is enabled. Target 1 is the Item To Use. |
| 9 | Ambush | `PKAM`? | `TESAmbushPackageData` | Optional | No | Yes | **Yes** | Location 1 is the Wait Location. Location 2 is the Ambush Location. Target 1 is the Ambush Target. |
| 10 | Flee (Non-Combat) | | - | Optional | No | Optional | No |
| 11 | Use Magic | | - | Optional | No | Optional | No |
| 12 | Sandbox | | - | No | No | Yes | No |
| 13 | Patrol | `PKPT` | `TESPatrolPackageData` | No | No | Yes | No | Location 1 is the Start Location. |
| 14 | Guard | | - | Yes | No | Optional | No | Location 1 is the Guard Location. |
| 15 | Dialogue | `PKDD` | `TESDialoguePackageData` | Yes | No | Optional | **Optional** | Location 1 is the Start/Wait Location. Location 2 is the Trigger Location. Target 1 has its type forced to 0. |
| 16 | Use Weapon | `PKW3` | `TESUseWeaponPackageData` | Yes | Yes | Yes | **Optional** | Location 1 is the Location. Location 2 is the Target Location. |
| 17 | Find2 | | - | Yes | No | Optional | No | Remapped to 0 (Find) on load, but still handled separately in some switch-cases. Location 1 is assumed. |
| 18 | Custom | `PKCU` | `TESCustomPackageData` | No | No | No | No |
| 19 | Custom Template | `PKCU` | `TESCustomPackageData` | No | No | No | No |
| 20 | Activate | | - | ? | No | ? | No |
| 21 | Alarm | | - | ? | No | ? | No |
| 22 | Flee | | - | ? | No | ? | No |
| 23 | Trespass | | - | ? | No | ? | No |
| 24 | Spectator | | - | ? | No | ? | No |
| 25 | React to Corpse | | - | ? | No | ? | No |
| 26 | Exit Chair | | - | ? | No | ? | No |
| 27 | Do Nothing | | - | ? | No | ? | No |
| 28 | In-Game Dialogue | | - | ? | No | ? | No |
| 29 | Surface | | - | ? | No | ? | No |
| 30 | Search for Attacker | | - | ? | No | ? | No |
| 31 | Avoid Player | | - | ? | No | ? | No |
| 32 | React to Destroyed Object | | - | ? | No | ? | No |
| 33 | React to Lobber Projectile | | - | ? | No | ? | No |
| 34 | Steal Warning | | - | ? | No | ? | No |
| 35 | Pickpocket Warning | | - | ? | No | ? | No |
| 36 | Movement Blocked | | - | ? | No | ? | No |
| 37 | Vampire Feed | | - | ? | No | ? | No |
| 38 | Cannibal Feed | | - | ? | No | ? | No |

A question mark in the header column indicates that code to save the package type exists, but the `TESPackage` loader doesn't *seem* to check for the given subrecord.

## Form contents

### Objects

#### Unique ID
Single-byte packdata identifier with `0xFF` used to indicate "none." These are essentially the "variables" in the package. The serialized data lists all values first, and then the packdata (unique IDs and metadata) to which those values are being supplied.

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
  * *Not selectable in the CK UI.*
  * ?
* LocCell *[found via RE]*
  * *Not selectable in the CK UI.*
  * A single interior cell?
* ObjectList
  * Value is CNAM?
  * Functionally, this is a handle. Some package procedures will search for objects matching some criteria, storing their results in an Object List.
* SingleRef
  * Value is PTDA. (Are there other possibilities or constraints? For example, is PTDA here constrained to package location types that produce a REFR, including reference aliases?)
* TargetSelector
  * Value is PTDA.
  * Functionally, this is a search criterion. Essentially, given the phrase "Find *X*, and store any matches in Object List *Y*," this is *X*. This means that TargetSelectors cannot be the run-on subjects of conditions.
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
* FollowTo
* ForceGreet
* Guard
* HoldPosition
* Hover
* KeepAnEyeOn
* LockDoors
  * Uses *LockUnlock* within CK?
* Orbit
* Patrol
* Pursue
* Sandbox
* Say
* Shout
* Sit
  * Uses *SitSleep* within CK?
* Sleep
  * Uses *SitSleep* within CK?
* Travel
* UnlockDoors
  * Uses *LockUnlock* within CK?
* UseIdleMarker
* UseMagic
* UseWeapon
* Wait
* Wander
