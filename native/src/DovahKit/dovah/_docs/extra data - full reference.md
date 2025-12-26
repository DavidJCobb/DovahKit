
# Extra data types

## Types serialized in data files

### A

#### `ExtraAction` (`XACT`: eXtra ACTion)

Parameter is a single four-byte non-pointer value.

#### Activate ref (`XAPD`, `XAPR`, `XAPC`)

**Used on:** REFR

#### `ExtraAlphaCutoff` (`XALP`: eXtra ALPha cutoff)

**Used on:** REFR

A numeric value used to modify an object's alpha-testing threshold. This is typically used for banners and carpets, to make them look more tattered.

#### `ExtraAmmo` (`XAMT`, `XAMC`: eXtra AMmo Type; eXtra AMmo Count)

**Used on:** REFR

#### `ExtraAttachRef` (`XATR`: eXtra ATtach Ref)

**Used on:** REFR

Intended to synchronize the movement of two refs, attaching the ref that has the data to some other ref that has a "specifically created animation." The CK wiki notes that this functionality is unused.

### C

#### `ExtraCellAcousticSpace` (`XCAS`: eXtra Cell Acoustic Space)

**Used on:** CELL

The default Acoustic Space.

#### Cell climate (`XCCM`: eXtra Cell CliMate)

**Used on:** CELL

#### `ExtraCellGrassData`/`ExtraGIDBuffer` (`XCGD`: eXtra Cell Grass Data)

**Used on:** CELL

#### `ExtraCellImageSpace` (`XCIM`: eXtra Cell IMagespace)

**Used on:** CELL

The default Imagespace.

#### `ExtraCellMusicType` (`XCMO`: eXtra Cell Music Override)

**Used on:** CELL

The default Music Type.

#### `ExtraRegionList` (`XCLR`: eXtra Cell List of Regions)

**Used on:** CELL

Lists the regions (`REGN`) that overlap the cell.

#### `ExtraCellWaterType` (`XCWT`)

**Used on:** CELL

#### `ExtraCharge` (`XCHG`)

**Used on:** REFR[weapon]

Defines the amount of enchanting charge remaining on an enchanted weapon. If absent, the weapon is fully charged.

#### `ExtraCollisionData` (`XTRI`)

The value is a collision layer ID.

#### `ExtraCount`

**Used on:** REFR[item]

Defines the quantity of an item placed in the game world. A single ref can be used to represent a group of items; this is most obvious with Ammo forms, wherein an `ExtraCount` of 1 produces a single arrow, while a plural count produces a quiver full of arrows.

### D

#### `ExtraDistantData` (`XLOD`)

### E

#### `ExtraEmittanceSource` (`XEMI`: eXtra EMIttance)

**Used on:** REFR

When applied to a ref, this controls the color of any light emitted by the ref (e.g. if the base form is a light, or if the model has glow effects). The emittance source can be a Light form or a Region form ("external emittance"); in the latter case, the light emitted will match that of the region chosen, varying with the time of day.

#### `ExtraEnableStateParent` (`XESP`)

**Used on:** REFR

#### `ExtraEncounterZone` (`XEZN`: eXtra Encounter ZoNe)

**Used on:** CELL[Interior], REFR

Sets the encounter zone that the cell or ref is associated with. If a ref lacks a zone, it inherits its zone from its containing cell.

### F

#### `ExtraFavorCost` (`XFVC`: eXtra FaVor Cost)

**Used on:** REFR

### G

#### `ExtraGlobal` (`XGLB`: eXtra GLoBal)

**Used on:** CELL, REFR

Associated with `ExtraOwnership`. Apparently unused; there's no UI you can use to set it. Probably related to how `ExtraOwnership` is presented for container inventory entries.

### H

#### `ExtraHeadtrackingWeight` (`XHTW`)

**Used on:** REFR

Unused.

#### `ExtraHealth` (`XHLT`)

**Used on:** Inventory entry

The health of an item that has condition (i.e. an armor or weapon), as an absolute value rather than a percentage. Value -1 is a sentinel for "unset."

#### `ExtraHealthPercent` (`XHLP`)

**Used on:** REFR[ARMO|WEAP]

Defines the health percentage of an item that has condition. This is the "Health" option exposed in the "Extra" tab of the "Reference" dialog.

Bethesda describes this option as unused, but in theory, you could still use values above 100% health to spawn tempered weapons and armor.

#### `ExtraHorse` (`XHOR`)

**Used on:** ACHR

Defines the horse actor that this actor will ride.

### I

#### `ExtraIgnoredBySandbox` (`XIS2`)

**Used on:** REFR

Marks a ref as being ignored by NPCs running a Sandbox AI procedure.

#### `ExtraLockList` (`XILL`)

**Used on:** CELL[Interior]

Specifies a FormList of NPCs, or a single NPC, who are allowed to freely enter and exit this interior, and lock and unlock its doors.

### L

#### `ExtraLevCreaModifier` (`XLCM`)

**Used on:** REFR[LVLN]

When set on a ref whose base form is a LeveledActor, this controls the difficulty (specifically the level) of the ref relative to the level of the relevant encounter zone. It also influences the color of the ref's marker model as displayed in the CK.

| Value | Color | Level | Level GMST |
| :-: | :-: | -: | :- |
| Easy | Green | ≤ 33% | fLeveledActorMultEasy |
| Medium | Yellow | 66% | fLeveledActorMultMedium |
| Hard | Orange | 100% | fLeveledActorMultHard |
| Very Hard | Red | 125% | fLeveledActorMultVeryHard |
| None | White | = player | |

When leveled actor refs use a difficulty modifier other than Easy or None, all refs in the same space that use the same modifier will have the same level.

#### `ExtraLeveledItemBase` (`XLIB`)

**Used on:** REFR[item]

LeveledItems are not base forms and cannot be placed directly in the game world. (This makes sense: it prevents an entire class of mishaps that could happen if, during runtime, a `REFR` somehow had its base form set to an `LVLI`.) This means that to have a leveled item spawn directly in the game world rather than in a container, you must:

* Place a regular item.
* Set the `ExtraLeveledItemBase` data on the ref (via the "Leveled Item" tab) to a LeveledItem.

When it comes time for the ref to spawn, it'll be replaced with whatever item is produced by the leveled list.

Bethesda has defined a suite of "dummy" forms in Skyrim.esm, e.g. `DummyPotion`, that they use for this purpose.

#### `ExtraLightData` (`XLIG`: eXtra LIGht)

**Used on:** REFR[LIGH]

Defines the lighting options set on a light ref: the fade, FOV, and so on.

#### `ExtraLinkedRef` (`XLKR`: eXtra LinKed Ref)

**Used on:** REFR

When used on refs, this defines a 1:1 map of Keyword forms to other ref forms. Additionally, you can map the absence of a Keyword (i.e. None) to a single ref as well.

Linked refs exist as a convenience mechanism and can be targeted by conditions, packages, and a few other mechanisms.

#### `ExtraLitWaterRefs` (`XLTW`)

#### `ExtraLocation` (`XLCN`)

When used on refs, this defines the ref's Persist Location.

#### `ExtraLocationRefType` (`XLRT`)

**Used on:** REFR

Indicates the ref's location ref type, for use in the Radiant Story system.

#### `ExtraLock` (`XLOC`)

**Used on:** REFR[CONT|DOOR]

Indicates that a container or door ref is locked, and describes the strength of the lock and the associated key, if any.

### M

#### `ExtraMapMarker` (`XMRK`)

**Used on:** REFR[MapMarker]

#### `ExtraMerchantContainer` (`XMRC`)

**Used on:** ACHR

Deprecated. Refers to a container ref.

In Fallout 3 and Fallout: New Vegas, this is applied to merchant NPCs. Items that they buy from the player are redirected to the specified container, so that the merchant doesn't become overburdened as a result of the player selling heavy items to them. Skyrim replaces this with a system wherein merchant job behaviors (and the containers used) are tied to Factions, which among other things means that when a merchant dies, another actor can take up their role.

#### `ExtraMultiBound` (`XMBO`: eXtra MultiBOund)

**Used on:** REFR

#### `ExtraMultiBoundRef` (`XMBR`: eXtra MultiBound Ref)

**Used on:** REFR

When refs are present in a cell that has roombounds/multibounds, a ref will only be rendered while the camera is in the multibound to which the ref belongs. By default, refs are assigned to the multibound that contains their pivot points. You can override this and assign a ref to a specific multibound via `ExtraMultiBoundRef`.

### N

#### `ExtraNavMeshPortal` (`XNDP`)

**Used on:** REFR[DOOR]

Associates a load door with a navmesh form in its containing cell, and a triangle index within that navmesh.

### O

#### `ExtraOcclusionPlaneRefData` (`XORD`)

**Used on:** REFR

#### `ExtraOcclusionShape` (`XOCP`: eXtra OCclusion Plane)

**Used on:** REFR

#### `ExtraOwnership` (`XOWN`)

**Used on:** CELL[Interior], REFR

Defines the owning faction or NPC of a reference or cell.

* The owner of a cell is the default owner of any refs within that cell which can be owned.

* The owner of a furniture is the only party allowed to use the furniture.

* The owner of an item is considered the item's rightful possessor; if the player is not an owner (nor friends with the owner), they will be regarded as a thief if they take the item.

This only defines the owning form. When the owning form is a faction and ownership is gated out to a minimum rank, that rank is stored via `XRNK`.

### P

#### `ExtraPackageStartLocation` (`XPSL`)

#### `ExtraPatrolRefData` (`XPRD`, `XPPA`)

**Used on:** REFR

Influences the behavior of any actor running a Patrol procedure, when they stop at this ref. You can control how long they wait at the ref, and direct them to play an idle and/or say a line of dialogue.

#### `ExtraPoison` (`XPSN`, `XPSC`: eXtra PoiSoN, eXtra PoiSon Count)

**Used on:** REFR[WEAP]

Indicates the poison type and dose applied to a weapon ref.

#### `ExtraPortal` (`XPTL`: eXtra PorTaL)

#### `ExtraPortalRefData` (`XPOD`: eXtra Portal Origin and Destination)

**Used on:** REFR

#### `ExtraPrimitive` (`XPRM`: eXtra PRiMitive)

**Used on:** REFR[CollisionMarker01]

Defines a collision primitive. If a ref has the `CollisionMarker01` base form and an `ExtraPrimitive`, then when it has its 3D loaded, said 3D will be synthesized from the `ExtraPrimitive` parameters.

### R

#### `ExtraRadius` (`XRDS`: eXtra RaDiuS)

**Used on:** REFR[LIGH, MapMarker]

When applied to map markers, this defines the marker radius (i.e. the distance that the player must come within to discover the associated location).

When applied to light refs, this defines the light's maximum radius.

In Fallout 3 and Fallout: New Vegas, when applied to a RadiationMarker this would indicate the radiation within which the player is irradiated.

This data can be set on any ref, and the CK will display the radius as a yellow-outlined sphere if View -> Light Radius is enabled.

#### `ExtraRagDollData` (`XRGD`, `XRGB`)

**Used on:** ACHR

Used to store a pose for actors that are flagged as Starts Dead.

#### `ExtraRandomTeleportMarker` (`XRTM`)

#### `ExtraRank` (`XRNK`)

**Used on:** CELL, REFR

The minimum faction rank that an actor must have in order to be regarded as sharing ownership of the cell or ref. Used in tandem with `ExtraOwnership` (`XOWN`), which defines the owning faction.

Value -1 is a sentinel for "unset."

#### `ExtraReflectorRefs` (`XPWR`: eXtra Plane Water Refs)

**Used on:** REFR[LIGH]

A list of water plane references that reflect this light.

#### `ExtraRoomRefData` (`XRMR+LNAM?+INAM?+XLRM[]`)

**Used on:** REFR[RoomMarker]

Defines data for a roombound marker: the number of linked rooms (`XRMR`), and the refs (other RoomMarkers) (`XLRM[]`) for those rooms. Can optionally also specif ya lighting template (`LNAM`) and an imagespace (`INAM`).

The subrecord order is exact. The game does not thoroughly check subrecord signatures.

### S

#### `ExtraScale` (`XSCL`: eXtra SCaLe)

**Used on:** REFR

Defines the scaling factor applied to a ref.

#### `ExtraSpawnContainer` (`XSPC`)

**Used on:** REFR

The intended effect seems to be to place the ref inside of the specified container's inventory. This seems to work for a copy of <i>The Totems of Hircine</i> located in `WhiterunJorrvaskrBasement`, at least for its initial spawn (commenters on the book's UESP talk page claim that they've personally seen it on a table rather than in its spawn container), but doesn't seem to work for Salvanius's cuirass and gauntlets (placed out of bounds and set to use a nearby chest as their spawn container; I didn't see them in its inventory when I checked it).

Theoretically, this feature could be used to add bespoke items to an otherwise typical container (e.g. Salvanius's armor being stored in a container whose base form is configured to just spawn leveled items), such that you don't have to create bespoke base Container forms instead.

### T

#### `ExtraTeleport` (`XTEL`)

**Used on:** REFR[DOOR]

When applied to a door ref, indicates that the ref is a load door. Given two load doors <var>A</var> and <var>B</var>, each will have `ExtraTeleport` data pointing to the other. The data for <var>A</var> will define the placement of the teleport marker on <var>B</var>'s side, and vice versa.

#### `ExtraTeleportName` (`XTNM`: eXtra Teleport NaMe)

**Used on:** REFR[DOOR]

The value is a Message form. This overrides the destination location name that is displayed when the player aims at the door.

In the CK's UI, this is shown in the "Teleport" tab of the "Reference" dialog, labeled "Teleport Loc Name."

#### `ExtraTimeLeft`

**Used on:** REFR

The value is a float.

### W

#### `ExtraWaterCurrentZoneData`

This consists of seven subrecords, distributed across a cell and its water current zone markers (i.e. child refs that use a water activator as their base form, and child refs that use `[STAT:0C4]WaterCurrentZoneMarker` as their base form).

| FourCC | Meaning | Appears on | Content |
| :- | :- | :- | :- |
| `XWCN` | eXtra Water Current Number | cell; water activator | Number of vector4s in `XWCU`. |
| `XWCU` | eXtra Water CUrrents | cell; water activator | Zero or more vector4s. When present, they appear to be the Linear Velocity and Angular Velocity. |
| `XCVL` | eXtra Current Velocity, Linear | water current zone marker | Vector3. Optional; not serialized if zero. |
| `XCVR` | extra Current Velocity, Rotational | water current zone marker | Vector3. Optional; not serialized if zero. |
| `XCZC` | eXtra Current Zone Cell | water current zone marker | Cell to which this water current zone marker pertains. Always followed by XCZA. |
| `XCZR` | eXtra Current Zone Ref | water current zone marker | A ref. Always followed by XCZA. |
| `XCZA` | eXtra Current Zone Action | water current zone marker | An unknown dword known not to be a form ID. |

Water current markers contain `XCZC`, specifying the cell to which they pertain, and `XCZA`.

Water activator refs contain `XWCN+XWCU`.

Cells contain `XWCN+XCWU`.

##### Seen cases

There's literally only one `WaterCurrentZoneMarker` used across the entire base game and DLCs. It has `ExtraPrimitive` data defining it as an almost flat box, 128x2x128wu, colored solid white. It hovers in the air far above any water, with no rotation, making it perpendicular to the ground. If you try to edit its Y-axis size in the CK, said dimension will be forced to zero: the CK really wants this marker to be a flat plane.

* `[CELL:00009BE0]EvergreenGroveExterior`
  * XWCN: 4
  * XWCU:
    * (0, 0, 0), 0.0F
    * (0, 0, 0), 0.0F
    * (-741.258545, -958.061096, -5.8811944), 4e-45F
    * (0, -1, 0), 0.91440004F
  * Refs
    * Water activator `00061916`
      * XWCN: 3
      * XWCU:
        * Linear velocity: (-0.9782, -0.695, 0), 0.0F
          * (Exactly matches the velocity defined on the ACTI's WTYP.)
        * Angular velocity: (0, 0, 0), 0.0F
        * (0, 0, 0), 0.0F
    * Water activator `0010cb02`
      * XWCN: 3
      * XWCU:
        * Linear velocity: (-0.9782, -0.695, 0), 0.0F
          * (Exactly matches the velocity defined on the ACTI's WTYP.)
        * Angular velocity: (0, 0, 0), 0.0F
        * (0, 0, 0), 0.0F
    * Water activator `0010cb03`
      * XWCN: 3
        * Linear velocity: (-0.9782, -0.695, 0), 0.0F
          * (Exactly matches the velocity defined on the ACTI's WTYP.)
        * Angular velocity: (0, 0, 0), 0.0F
        * (0, 0, 0), 0.0F
    * Water current zone marker `0010B00A`
      * XCZC: `[CELL:00009BE0]EvergreenGroveExterior`
      * XCZA: 0

#### `ExtraWaterData`

#### Water environment map (`XWEM`)

**Used on:** CELL[Interior]

The value is a string.

## Scrapped types

### Horse (Legacy) (`XHRS`)

**Formerly used on:** REFR  
**Leftover from:** Oblivion

A pre-modern equivalent to `XHOR`. It functioned the same way, accounting for Oblivion dividing actors into "Characters" and "Creatures" (i.e. its value was an `ACRE`, an Actor Creature ref).

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### Ignored By Sandbox (Legacy) (`XIBS`)

**Formerly used on:** REFR  
**Leftover from:** Fallout 3/New Vegas

An empty subrecord whose mere presence acted as a flag. It has been superseded by `XIS2`.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded (i.e. no extra-data object is constructed; nothing becomes present).

### Linked Decals (`XDCR`)

**Formerly used on:** REFR  
**Leftover from:** Fallout 3/New Vegas

This seems to have been similar to linked refs, but storing a list of decal-emitter refs, rather than a map of keywords to arbitrary refs. One `XDCR` would be present per emitter.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### Persistent Cell, Interior (`XPCI`)

**Formerly used on:** REFR  
**Leftover from:** Morrowind/Oblivion

Its value was a `CELL`.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### Radiation (`XRAD`)

**Formerly used on:** REFR[RadiationMarker]  
**Leftover from:** Fallout 3/New Vegas

A float indicating rads per second inflicted on actors within this ref's radius. The CK still has UI for it in the form of the "Radiation" option in the Reference dialog's "Extra" tag.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### Radio (`XRDO`)

**Formerly used on:** REFR  
**Leftover from:** Fallout 3/New Vegas

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### SpeedTree seed (`XSED`)

Its value used to be a single integer; xEdit claims it was a `uint8_t`.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### `ExtraSoul` (`XSOL`)

**Formerly used on:** REFR  
**Leftover from:** Oblivion

A soul-size enum. The CK still has UI for it in the form of the "Soul" option in the Reference dialog's "Extra" tag.

In both the game and CK, the `CELL` and `REFR` loaders forward it to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

### Unknown

* XCET
  * Seen in Fallout: New Vegas. A single-byte value, meaning unknown. Used on `CELL`s.
* XEDL
* XENC
* XLMB
* XNVP
* XROO
* XUSE
* XWCS
* XWLT
* XWNT

In both the game and CK, the `CELL` and `REFR` loaders forward subrecords with these signatures to the extra-data loader, but that loader doesn't handle it, so the data is never actually loaded.

## Run-time-only types

Currently outside the scope of our documentation.