
## Improve type-safety

Right now, `DovahKitCore`'s accessors for modifying the value of a game setting are not type-safe. This is because the backend doesn't do any type-checking: it takes a `dovah::game_setting_value` (basically an untagged union) as a value, and the exception for failing to set a setting does not include an enum code for type mismatches.

We should fix this: APIs for changing a setting's value should take a `std::variant`, and we should verify its type against the type of the setting (as dictated by its Hungarian notation prefix).

(Once this change is made, we may want to update the nascent "game settings subsystem" in the frontend to take advantage of this.)