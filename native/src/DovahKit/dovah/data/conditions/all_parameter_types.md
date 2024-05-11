
## Notes

* **`Alias`:** though some conditions require a ref alias or a loc alias specifically, the CK makes no attempt to ensure you are providing an alias of the correct type.
* **`BaseForm`:** also includes a few other form types even though GetIsID et. al could never run on them; probably a mistake on Beth's part
* **`Cell`:** testing in CK indicates that only interiors are allowed; named exteriors are not
* **`EquipType`:** this enum was removed from the game and is only used in one condition, which is both deprecated and broken in two different ways. the CK shows an empty drop-down when trying to choose a value.
* **`Furniture`:** TODO: xEdit defs say this can also take a FLST; double-check that and implement if so
* **`KnowableForm`:** Reverse-engineer conditions that use this; if they're not strict about form type, we don't need to be either, since the "Is Known" flag is common to all form types IIRC
* **`MiscStat`:** the values of this enum are CRCs of misc stat name strings