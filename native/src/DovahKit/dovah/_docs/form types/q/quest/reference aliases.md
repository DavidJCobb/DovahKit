
# Reference aliases

In Skyrim and the Creation Kit, reference alias fill parameters are stored as a tagged union, with the tag being a member of the `BGSRefAlias` object. The possible options are:

* Conditions
* Forced (i.e. pre-assigned reference)
* From Sibling Alias
* From Event
* Created
* From External Alias
* Unique Actor
* Near Alias

An alias defaults to "Conditions" if it has no fill-type subrecords and at least one condition, or to "Forced" (with the ref being None) otherwise.

DovahKit uses its own tagged union (via `std::variant`), but with more granular types. The mapping is as follows.

| DovahKit type | Bethesda type | Trigger |
| :- | :- | :- |
| `ref::preassigned` | `ForcedFillData` | Subrecord |
| `ref::unique_actor` | `UniqueActorFillData` | Subrecord |
| `ref::at_location_alias` | `FromAliasFillData` |  Subrecord |
| `copy_external_alias` | `FromExternalFillData` | Subrecord |
| `ref::create` | `CreatedFillData` | Subrecord |
| `ref::find_anywhere` | `Padding` | No subrecords + at least one condition + no "limit to loaded area" alias flag |
| `ref::find_in_loaded_area` | `Padding` | No subrecords + at least one condition + "limit to loaded area" alias flag |
| `ref::find_from_event` | `FromEventFillData` | Subrecord |
| `ref::find_near_alias` | `NearAliasFillData` | Subrecord |
