
# Perks

We need to make a better distinction between...

* **Perk entry points:** Opportunities to intercept some sort of game behavior.
* **Entry point functions:** The specific kind of interception we are performing, e.g. a numeric calculation or the replacement of a string.
* **Entry point result types:** The data type produced by an entry point function.
* **Entry point parameter types:** The type of a set of parameters provided to an entry point function.

Currently, these are represented as follows:

* **Perk entry points** are represented via the `perk_entry_point` enum, and each enum value is annotated by `perk_entry_point_info` structs in the `all_perk_entry_point_info` array.
* **Entry point functions** are represented via the `entry_point_function` enum.
* **Entry point result types** are represented via the `entry_point_value_type` enum. **This identifier isn't ideal, and worse still, result types are improperly stored per `perk_entry_point_info` rather than being associated with `entry_point_function`s.** This association works for the perk entry points defined in Skyrim, but doesn't match how things actually work. (For example, if hypothetically Bethesda added a "Delay Activation by Seconds" function to the "Activate" entry point, then that function would have a different result type from "Add Activate Choice.")
* **Entry point parameter types** are represented via the `entry_point_function_type` enum. **This identifier isn't ideal.**
