
A dialogue branch specifies its owning quest via the `QNAM` subrecord. However, the game accidentally performs the local-to-global form ID conversion incorrectly: it performs the conversion in `BGSDialogueBranch::LoadForm`, which it shouldn't be doing; and then the ID is converted again in `BGSDialogueBranch::InitItem`, which is where form ID conversions should happen. This means that the form ID gets converted twice and garbled.

When the game performs a local-to-global conversion, if the input form ID is higher than the highest load order prefix in the file, then it is clamped to that prefix. Consider, then, the following load order, with `TestFile.esp`'s masters marked, and with local and global load order prefixes listed:

| Master | L. | G. | Filename
| ---- | -- | -- | - |
| Yes  | 00 | 00 | Skyrim.esm |
| Yes  | 01 | 01 | Update.esm |
| Yes  | 02 | 02 | Dawnguard.esm |
| No   | 03 |    | MyCoolFile.esp |
| No   | 04 |    | SomethingWeird.esp |
| No   | 05 |    | RandomMod.esp |
| Yes  | 06 | 03 | SomeFramework.esp |
| Self | 07 | 04 | TestFile.esp | 

If `TestFile.esp` defines a new dialogue branch whose owning quest is in `Skyrim.esm` or `Update.esm`, then that reference will be interpreted correctly because those two files have the same global load prefix as their local load prefix within `TestFile.esp`. However, if `TestFile.esp` defines a new dialogue branch whose owning quest is in `SomeFramework.esp`, then the game will mangle the quest's form ID when redundantly converting it: first, it will be converted from 03xxxxxx to 06xxxxxx; then, it will be converted again. Because 06 is out-of-bounds within the local file list (the highest local load prefix is 04), the form ID will be placed within `TestFile.esp` itself, thereby being moved to 07xxxxxx.

If `TestFile.esp` happens to have a quest with that form ID, then that quest will wrongly become the owning quest for this dialogue branch; otherwise, the dialogue branch will have no owning quest, which is also wrong. The exact impact that this has on the game is not known at this time.

DovahKit doesn't attempt to mimic this load error, because frankly, that would be insane. However, we're obligated to warn about it.

As for when this error would occur, and when it would break things? Well, most of the time, it shouldn't actually do any damage. If you're editing dialogue in a master, you only need to override the `DLBR` if you change a branch's owning quest (which I don't think the CK lets you do, and which you have no reason to do), if you change its type or flags (again, useless), or if you change its starting topic. If you're creating a new dialogue branch whose owning quest belongs to Skyrim or Update, then you'll avoid the issue because the quest's form ID will survive a redundant conversion (unless Skyrim and Update aren't the first two files in your mod's master list, but why the hell would you ever do that?). If your dialogue branch's owning quest belongs to the same file that supplies the branch (i.e. dialogue in your own mod), then you'll be fine as well: the quest form ID will get mangled by the redundant conversion, but the game will fall back to placing it within your own file.

Where you're at risk is if you override a dialogue branch in another mod or in a DLC file, or if you create a dialogue branch whose owning quest is in another mod or in a DLC file.