
## Improve type-safety

Right now, `DovahKitCore`'s accessors for modifying the value of a game setting are not type-safe. This is because the backend doesn't do any type-checking: it takes a `dovah::game_setting_value` (basically an untagged union) as a value, and the exception for failing to set a setting does not include an enum code for type mismatches.

We should fix this: APIs for changing a setting's value should take a `std::variant`, and we should verify its type against the type of the setting (as dictated by its Hungarian notation prefix).

(Once this change is made, we may want to update the nascent "game settings subsystem" in the frontend to take advantage of this.)


## Full access

For each game setting, the `file_load_order` maintains one `loaded_game_setting` object which identifies the source file (needed so we can resolve localized strings) and the game setting value. However, the `file_load_order` only grants access to the last-loaded `loaded_game_setting` for a given setting name.

This interferes somewhat with the "Oops! All ITMs" debug test, which wants to know whether a given form (or ideally game setting) was defined or overridden in some desired file. We can't tell that a given GMST has a record in a given file unless that file supplied the winning record specifically, because we can't see *all* of the source files for a game setting.

It'd be nice to re-think how GMSTs are retained in memory. It should be straightforward for the outside world to get a list of all source stubs for a game setting. (Separately, just as I want to have stubs stored by a `form_collection` so that the future `active_load_order` is less of a god object, I should also have a `game_setting_collection` for loaded GMSTs.)
