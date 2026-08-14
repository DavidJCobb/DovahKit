
# Voicetypes valid for a TopicInfo

When you view a Response in any TopicInfo, a listpane will show all valid voicetypes for that info, and the path to the Response's sound file for that voice. The voicetypes listed are dependent on the conditions applied to that TopicInfo.

The following conditions can add or remove voicetypes from the listview:

* `GetIsAliasRef` checks the specified alias on the condition's owning quest. If the alias fills from a pre-placed ref, a unique actor, or a created ref, and if the ref is an actor, then the voicetype of the ActorBase[^resolve-template] is used.
* `GetInFaction` uses the voicetypes of all ActorBases[^resolve-template] that are listed in the given faction within the CK, regardless of rank (even -1).
* `GetIsID` uses the voicetype of the specified ActorBase[^resolve-template] or Talking ACtivator.
* `GetIsRace` uses the voicetypes of all ActorBases[^resolve-template] of the specified race.
* `GetIsSex` uses the voicetypes of all ActorBases[^resolve-template] of the specified sex.
* `GetIsPlayableRace` uses the voicetypes of all ActorBases[^resolve-template] who belong to a Playable-flagged race.
* `GetIsVoiceType` is self-explanatory.

[^resolve-template]: When the game examines an actor, it checks if that actor has a template with the "Traits" template flag set. If so, it follows the "Traits" template chain all the way to the end, and uses the stats on the final found actor.

The Creation Kit shortcuts the logic for evaluating conditions. In general, if the only conditions on your info serve to exclude forms, then nothing will be listed; for example, if the only condition is `GetIsID(Actor01) != 1`, then nothing gets listed. The Creation Kit *does try* to process or-groups, such that the following condition lists correctly produce an empty listview:

* `GetIsID(Actor01) && GetIsID(Actor02)`
* `GetIsID(Actor01) && (GetIsID(Actor02) || GetIsID(Actor03))`

But the following condition list incorrectly produces an empty listview as well:

* `GetIsID(Actor01) && (GetIsID(Actor01) || GetIsID(Actor02))`

And these conditions will *not* result in an empty listview, and will instead list the mentioned actor:

* `GetIsID(Actor01) && !GetIsID(Actor01)`

And these conditions will result in *every* actor's voice being listed:

* `GetIsID(Actor01) || !GetIsID(Actor01)`

I tried reverse-engineering the code that the CK uses to check these conditions, and it's... strange. They seem to build multiple hashmaps of forms, and they maintain a bitvector that apparently corresponds to what voicetypes are eligible for the TopicInfo being tested. I always struggle with reading hashmap operations in a disassembler, but even without *that* problem, I also could only even find where it handles `GetIsSex`, `GetIsAliasRef`, `GetIsID`, and `GetIsVoicetype`. All of the above results are the result of empirical testing alone.

The upshot of all of this is that it's... not easy, and probably not even worthwhile, to match how the Creation Kit generates a list of eligible voicetypes for a TopicInfo. I think it'd be better to just have DovahKit list all voicetypes unconditionally, particularly given that one can use conditions that dynamically include voicetypes during play (e.g. `GetInFaction`).