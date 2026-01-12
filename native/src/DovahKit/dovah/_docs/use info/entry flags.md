
# Use info entry flags

DovahKit stores use info bidirectionally on form stubs. Each stub has an "inbound" and "outbound" map of form IDs to `use_info::entry` structs, each of which contains:

* A pointer to the other-side form stub.
* A refcount, indicating how many different times the user-stub uses the used-stub.
* A set of use info "entry flags."

Use info entry flags exist to mark unique 1:1 uses that have special meaning. Given a user form and a used form, if the use info between these forms has a given flag set, then it means that exactly one use (of the used form by the user form) pertains to that flag.

For example, a single ref can only have one base form (`REFR/NAME`), and it is extremely useful to be able to quickly look up a ref's base form without having to fully load all of the ref's data. Thus, the ref's use of its base form is marked with a use info flag, so that we can find the base form via the ref's use info.[^get_base_form]

[^get_base_form]: A helper function exists to perform this lookup: <code>dovah::<wbr/>form_stub_helpers::<wbr/>get_base_form(<wbr/>form_stub& ref)</code>. As you might expect, it just traverses over the ref's outbound uses, searching for one which has the <code>dovah::<wbr/>use_info::<wbr/>entry_flags::<wbr/>reference::<wbr/>base_form</code> flag.

## Limitations

Use info entry flags can only be used to mark unique/one-to-one uses. For example, we can't introduce a use info entry flag for a ref's linked refs, because a ref can have an arbitrary number of linked refs (limited only by the game's memory budget and form ID space, really), and the same target form could potentially appear in multiple linked ref entries (whether because the same ref is linked via multiple keywords, or because the data is malformed). If we wanted to be able to quickly identify what uses are of linked refs, we'd need an entire *counter* on every use info entry dedicated just to linked refs.

Similarly, separate uses need separate flags even if, in well-formed data, they'd be mutually exclusive. A dialogue topic should never list the same form as both its owning quest and its owning dialogue branch, so hypothetically, one may wish to use a single "owner" use info entry flag to represent both of these uses; but a malformed topic *could* list the same form for both uses, and DovahKit attempts to preserve type-mismatched uses, so they need different flags.

## Currently defined flags

As of this writing, DovahKit tracks the following use info flags, sorted by form type, with the following rationales.

* All forms
  * **Parent/child record relationship.** Technically, only forms that can be child records[^child-forms] need this. However, making this a form-type-specific flag would be burdensome within the backend (i.e. having to branch all uses of this flag by form type).
* All forms with extra-data
  * **xxxx/XEZN (Encounter Zone):** To optimize gathering a Location's contents.[^location-gather]
  * **xxxx/XLCN (Location):** To optimize gathering a Location's contents.[^location-gather]
  * **xxxx/XTEL+0x00 (Teleport Destination Door):** For more optimal processing of load doors.
* ACTI (Activator)
  * **ACTI/WNAM (Water Type):** For rapidly checking whether refs need persistence. If the base form is an Activator (or subclass) with a water type, then the CK says it needs persistence.
* ARMO (Armor)
  * **ARMO/TNAM (Template Form):** No specific reason as yet, other than that it seems potentially useful.
* DIAL (Topic)
  * **DIAL/BNAM (Parent Branch):** Used when deleting the parent branch, as part of an option to automatically delete all dialogue forms therein. Also used by the `DIAL` UI to prevent adding multiple topics with the same subtype to the same branch/quest.
  * **DIAL/QNAM (Parent Quest):** Used when deleting the parent quest, as part of an option to automatically delete all dialogue forms therein. Also used by the `DIAL` UI to prevent adding multiple topics with the same subtype to the same branch/quest.
* DLBR (Dialogue Branch)
  * **DLBR/QNAM (Parent Quest):** Used when deleting the parent quest, as part of an option to automatically delete all dialogue forms therein.
* ECZN (Encounter Zone)
  * **ECZN/DATA+0x04 (Location):** To optimize gathering a Location's contents.[^location-gather]
* FURN (Furniture)
  * **FURN/WNAM (Water Type):** Same as ACTI/WNAM. (FURN is a subclass of ACTI, but the way DovahKit handles forms doesn't allow for subclasses unless they add no new fields, so we have to give FURN its own flag list.)
* LCTN (Location)
  * **LCTN/PNAM (Parent):** For rapidly querying location hierarchies, and checking if some location is inside of some other location. This would also make it easier to show a tree of locations in the Object Window.
* NPC_ (ActorBase)
  * **NPC_/TPLT (Template Form):** For rapidly gathering all template actors that influence the contents of a templated actor. Templating can be daisy-chained, and different properties can be inherited by each successive actor, so you may need to consult several templates to determine the final properties of some templated actor.
* REFR (ObjectReference)
  * All extra-data use info flags
  * **REFR/NAME (Base Form)**
* REGN (Region)
  * **REGN/WNAM (Parent World):** This would make it faster and easier to filter a list of regions by the worldspace they're allowed to be used in.
* SCEN (Scene)
  * **SCEN/PNAM (Parent Quest):** For rapidly locating all scenes owned by a given quest, and for deleting scenes when their owning quest is deleted.
* WEAP (Armor)
  * **WEAP/CNAM (Template Form):** No specific reason as yet, other than that it seems potentially useful.
* WRLD (Worldspace)
  * **WRLD/WNAM (Parent World):** This would make it easier to show a tree of worldspaces in the Object Window, if we wanted to do that.
  * **WRLD/XEZN:** To optimize gathering a Location's contents.[^location-gather]
  * **WRLD/XLCN:** To optimize gathering a Location's contents.[^location-gather]

[^child-forms]: CELL, LAND, NAVM, REFR, and subclasses of REFR.

[^location-gather]: LCTN forms need to maintain lists of: all unique actors that use the location as their Persist Location; all "special refs" (i.e. anything with a LocRefType) placed in a cell for which the LCTN is the immediate containing location (as determined by the cell's Encounter Zone or Location, or those of its parent worldspace); and some similar odds and ends. Additionally, the LCTN UI needs to go a step beyond, and show all unique actors that are placed in one of the location's cells *or* that use it as their Persist Location. It'd be really, really nice to be able to gather this information using Use Info alone.

## Implementation notes

### Enums are defined mostly independently

One of the goals of this system was to implement it such that if a given form type would benefit from having use info entry flags, its flags can be defined independently of everything else in the system: we should never have to recompile everything that touches use info in order to add or edit a single form type's flag enum. Accordingly:

* Use info entries store flags as an integral-typed value. Even if I had a template that could convert enums to bitmasks, I wouldn't use it here.

* We identify use-info-entry-flag enum types, and define metadata about them (such as what form types they are for), using template specialization. For example, `template<typename T> struct is_entry_flag_type` is defined to produce `false` for all types by default, and the header that defines a given use-info-entry-flag type will also specialize `is_entry_flag_type` for that type so that it produces `true`.

  Compare this to the idea of `using all_use_info_entry_flag_types = cobb::class_array<...>` or similar, and then testing `all_use_info_entry_flag_types::contains<T>`. Even if the class-array were defined using forward-declarations of the enums so that we could edit their individual contents freely, it would still have a large blast radius (i.e. force us to recompile a lot of stuff) if we ever wanted to add a flag enum for another form type.

### ...Except for the foundational enums

The two exceptions to the above rule are:

* `dovah::use_info::entry_flags::base`, which defines use info entry flags that need to be available on all record types (either because they're applicable to all forms, or because the innermost machinery in DovahKit runs simpler or more efficiently if it doesn't have to constantly branch on form types to know whether to even check the flag).
  
  The header which defines this flag also defines a constant, `first_form_type_specific_flag`, which every form-type-specific flag enum should use as the value of its first member.

* `dovah::use_info::entry_flags::base_extra_data`, which defines use info entry flags that pertain to extra-data (e.g. `ExtraEncounterZone`, `ExtraLocation`). Similarly to how all form-type-specific flag enums continue from the end of `base`, form types which can *have* extra data need to have their flag enums continue from the end of `base_extra_data` by setting their first member to `first_non_extra_data_flag`.
  
  This enum needed to exist because there's no *clean and sane* way to propagate the form type that includes an extra-data list down through that list and into all of its extra-data; there's no clean and safe way to have e.g. `ExtraLocation` use particular flags in a "cell" enum or a "reference" enum based on what form type contains the extra-data list. Easier if we just say, "These flags apply across any forms that have extra-data." This of course also means that any flags used in extra-data are exposed for all forms that can have extra-data, even if a form would not normally (or usefully) have a given extra-data type (e.g. `CELL/XTEL` is technically valid but meaningless; still, there has to be a flag for that in order for there to be a flag for `REFR/XTEL+0x00`).

## Using these flags for fast lookups

In general, you can rely on functions in `dovah::form_stub_helpers` for these sorts of fast lookups.

* `for_each_inbound_use_with_flag` can be given a used-form and will find all user-forms that use the used-form with the given use info flag. For example, given a quest, you can find all of the scenes which are owned by that quest by looking up inbound uses with the `dovah::use_info::entry_flags::scene::parent_quest` entry flag.

* `get_unique_outbound_use` can be given a user-form and will find the used-form, if any, that has the given flag.

Both of those functions validate the type of the user-form (because that's necessary in order to interpret the flags properly, since the flags-mask is effectively a union discriminated by the user-form's form type), but not the type of the used-form. For example, if you use `get_unique_outbound_use` to look up a ref's base form: if the "ref" is actually an Activator, then the function will return `nullptr` immediately; but if a Reference claims that its base form is a Keyword, then the function will return that Keyword's form stub.

Several functions are present in `dovah::form_stub_helpers` which are built off of the above two functions.