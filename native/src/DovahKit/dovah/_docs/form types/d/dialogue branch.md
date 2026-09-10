
# Dialogue branch

If a dialogue topic is an individual remark you can say to an NPC, and a topic info is a reply the NPC can give, then a dialogue branch is a collection of topics representing a single "thread" of conversation.

## Noes

### Owning quest edge case

Dialogue branches specify their owning quest via the `DLBR/QNAM` subrecord. However, Bethesda made a strange mistake when programming the loader for `BGSDialogueBranch`.

In Bethesda's codebase, forms store uses (of other forms) as a `union` of a record ID and a pointer. The virtual member function `TESForm::Load` pulls record IDs from a data file and stores them; after all forms are loaded into memory, the virtual member function `TESForm::InitItemImpl` is used to resolve those record IDs to form IDs, and then to replace them with a pointer to the desired form.

For some reason, `BGSDialogueBranch` performs part of this process for its owning quest pointer early, in `BGSDialogueBranch::Load`, where it resolves the record ID to a form ID. Then, in `BGSDialogueBranch::InitItemImpl`, it performs the full process. The effect is that the owning quest's 'record ID is resolved to a form ID twice, which scrambles it in many situations. Specifically, given some file *Foo.esp* which defines or overrides a dialogue branch: if the load order prefix of the target quest's record ID is not identical to the load order prefix of the target quest's form ID, then the lookup will fail.

An example may make this easier to understand. Consider the following load order, with files' masters indicated with nested bullets:

* 00 Skyrim.esm
* 01 Update.esm
  * 00 Skyrim.esm
* 02 Dawnguard.esm
  * 00 Skyrim.esm
* 03 Hearthfires.esm
  * 00 Skyrim.esm
* 04 Dragonborn.esm
  * 00 Skyrim.esm
* 05 MyCoolMod.esp
  * 00 Skyrim.esm
  * 01 Update.esm
  * 02 Dragonborn.esm

If *MyCoolMod.esp* attempts to add a dialogue branch to a quest in Dragonborn, it will identify that quest using a record ID with prefix `02`, since Dragonborn is the mod's third master'. When this is loaded, `BGSDialogueBranch` will resolve it to a form ID with prefix `04`, since Dragonborn has that index in the player's full load order. Then, `BGSDialogueBranch` will try to resolve this form ID *again*, treating it as a record ID. However, `04` is out of bounds; the mod doesn't have that many masters; and when the game resolves a record ID that is out of bounds, it acts as though that record ID had the prefix of the containing file. Thus, the form ID's prefix is changed to `05`, the load order position of the mod itself, and the dialogue branch fails to specify the intended quest as its owner.

So by default, it's unsafe for a mod to try to add a new dialogue branch to a quest defined in one of its masters. However, there are cases where it *is* safe, else all branches in all DLCs would be broken. Let's look at one of those cases:

* 00 Skyrim.esm
* 01 Update.esm
  * 00 Skyrim.esm
* 02 Dawnguard.esm
  * 00 Skyrim.esm

If Dawnguard wants to add a branch to a quest defined in the base game, then it'll use a record ID with prefix `00`. This will map to a form ID with prefix `00`: Skyrim.esm is the first entry in both Dawnguard's master list and the player's load order. Then, the form ID will be resolved (as if it were a record ID) again, but this just means that `00` is converted into `00` a second time. Thus, the intended quest is still used.

This means that it's safe for a mod to add a new dialogue branch to a quest defined in one of its masters, in the following cases:

* The mod is adding to Skyrim.esm, and that file has the correct[^correct-positions-in-master-list] position within the mod's master list.

* The mod is adding to Update.esm; the mod's master list includes both Skyrim.esm and Update.esm; and those files have correct[^correct-positions-in-master-list] positions within the mod's master list.

* The mod is adding to Dawnguard.esm; its master list includes all of Skyrim.esm, Update.esm, and Dawnguard.esm; and these files have correct[^correct-positions-in-master-list] positions within the mod's master list.

* The mod is adding to Hearthfires.esm; its master list includes all of Skyrim.esm, Update.esm, Dawnguard.esm, and Hearthfires.esm; and these files have correct[^correct-positions-in-master-list] positions within the mod's master list.

* The mod is adding to Dragonborn.esm; its master list includes all of Skyrim.esm, Update.esm, Dawnguard.esm, Hearthfires.esm, and Dragonborn.esm; and these files have correct[^correct-positions-in-master-list] positions within the mod's master list.

In all other cases, it's unsafe: it would only work if the end user has a load order that perfectly matches the mod's master list. For example, if the mod adds a quest to Dragonborn, and puts Dragonborn at the correct[^correct-positions-in-master-list] position within its master list, but doesn't list Dawnguard and Hearthfire as masters, then the quest record ID will be mangled when it is double-resolved (`02` -\> `04` -\> out-of-bounds). If the mod adds a quest to Dragonborn, doesn't list Dawnguard and Hearthfire as masters, but also doesn't put Dragonborn at the correct position -- if, instead, the mod puts two random other mods between Update.esm and Dragonborn.esm in its master list -- then the record ID will be resolved properly if the user has all DLCs installed, but the mod itself does nothing to *require* that the user have the other DLCs installed (they're not among the mod's masters).

The rules above can be generalized thusly:

```
// given: `file`     = the mod
// given: `quest_id` = the record ID of the quest (relative to `file`'s master list)

if (quest_id == 0)
   return true; // PASS: a None form ID won't break

let prefix = quest_id >> 0x18; // load order prefix
if (prefix == file.masters.length)
   return true; // PASS: file is specifying one of its own quests

let bethesda = [
   "Skyrim.esm",
   "Update.esm",
   "Dawnguard.esm",
   "Hearthfires.esm",
   "Dragonborn.esm"
];

if (prefix >= bethesda.length)
   return false; // FAIL: file is specifying a quest defined in a mod

for (let i = 0; i <= prefix; ++i) {
   if (file.masters[i].name != bethesda[i]) {
      return false;
      // FAIL: not a base/DLC file
      //       OR not all preceding base/DLC files listed as masters
   }
}

return true; // PASS
```


[^correct-positions-in-master-list]: The game and CK are hardcoded to force the base game and its DLCs to have the correct ordering within the player's load order: Skyrim.esm must always be the first file; Update.esm, the second; and then after those, whichever DLCs are installed will appear in the order Dawnguard, Hearthfire, and Dragonborn. Implied here is that the game will never allow other files to be placed between these.