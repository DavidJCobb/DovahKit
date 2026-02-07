[rule-of-one]: ./../../glossary%20-%20game%20data.md#Rule-of-One

# Topic Infos

Within Bethesda's dialogue engine, a <dfn>topic</dfn> is in most cases something that the player can say to an NPC, and a <dfn>topic info</dfn> (commonly just called an <dfn>info</dfn>) is in most cases a response that an NPC can give to the player.

Each topic has a list of infos, and each info can have conditions which control whether the info is a valid response at the current time. The player will only be able to see a topic if at least one of its infos isn't ruled out by conditions. When the player selects that topic, the game will generally take the first info whose conditions match, and use that as the NPC's response.[^random-info]

[^random-info]: Infos can be flagged as "random." If the first valid info is so flagged, then the game will collect it and any other random infos it sees until it encounters a non-random info or a "random end" info. Then, it'll pick one of those infos at random.

## Notes

In general, there's a lot of jank involved in the loading of `INFO` records and the management of their relationship to their parent topic.

*  Normally, when an `INFO` override is seen, `TESTopicInfo::ClearData` will clear the info's conditions, "link to" topic list, and responses, and will remove the topic from its parent info. All of this happens before the record's content is loaded. <a name="note-partial-not-cleared">However, `ClearData` won't do any of these things if the override is flagged as "partial."</a> This results in some nasty [edge-cases](#Edge-cases).

  * `TESTopicInfo::InitializeData` is called soon after `ClearData`, before the contents of the record are loaded, and will clear the info's pointer to its owning topic, and reset the info's cached index (within its owning topic) to `0xFFFF`, regardless of how the override is flagged. This is relevant for the explanation below.

* `TESTopicInfo::LoadForm` aborts immediately and returns `false` if the info does not have a parent record.

* When `TESTopicInfo::LoadForm` has finished consuming an `INFO` record, it invokes a member function on the parent `DIAL` record's `TESTopic` instance. We'll call this function `TESTopic::OnInfoLoaded`. This member function is responsible for adopting the info and placing it appropriately within the topic's child list based on the presence and value of `INFO/PNAM`, the subrecord which identifies an info's intended previous sibling.

  To understand how this works, it's helpful to define a few terms and point out one fact. Let the <dfn>current info</dfn> be the info whose `INFO` record is being loaded, and let the <dfn>current topic</dfn> be the topic whose `DIAL` record is the parent record of that `INFO` record. Be aware, also, that every info caches its index within its parent topic's info list, storing that index as a `uint16_t`.

  * If the subrecord is zero (i.e. None), then the current info is placed at the start of the current topic's info list.

  * If the subrecord is missing, then it's treated as though its value were `0xFFFFFFFF`.

  * If the subrecord is `FFFFFFFF`, then the info is appended to the end of the current topic's info list.

  * If the subrecord is any other value, then it specifies a preferred previous-sibling info.

    Begin by checking the current info's cached index. If that index exceeds the bounds of the current topic's info list, then it is assumed to be `0xFFFF` i.e. the current info was never placed in a parent topic. Otherwise, the current topic is assumed to belong to the current topic; that index within the current topic's info list is set to `nullptr`.

    Next, search the current topic for the info that was referenced by the `PNAM` subrecord. If a match is found, insert the current info into the current topic's info list, placing it after the match. Otherwise, place the current info at the start of the list.

  No matter how the info is placed, its parent pointer is set to the current topic, and the cached indices of all infos inside of the parent topic are updated.

### Edge-cases

* As mentioned [above](./#note-partial-not-cleared), a "partial" `INFO` override will not clear data loaded from any losing records. <a name="edge-case-partial-not-cleared">This violates the [Rule of One][rule-of-one].</a> The effect of this behavior is that for any info wherein the <var>n</var> records preceding the winning override are flagged as "partial," the final loaded conditions, "link to" topics, and responses will consist of data coalesced across those <var>n</var> records and the winning override, with each record among those appending to the previously loaded data.

  The consequence for DovahKit is that we need to bifurcate all of those lists. When an info is defined in the active file, we can only edit the conditions, etc., that were loaded from the active file, and not those loaded from any immediately preceding "partial" overrides.

* In turn, if a partial `INFO` override re-parents an info under a different topic, the info will actually be present in both its former parent topic and its new parent topic. Recall that `TESTopicInfo::ClearData` is expected to remove the info from its parent, but won't if the info record is "partial;" and observe that regardless of where the info thinks it is, `TESTopic::OnInfoLoaded` only sets the info's parent pointer, without trying to remove the info from whatever parent it is inside of.

  This means that allowing a "partial" info to be re-parented is ill-formed.

  DovahKit does not attempt to emulate this bug.

* Worse edge-cases also occur when loading an `INFO` record that has been flagged as "deleted." The game doesn't call `ClearData` or `InitializeData` for "deleted" overrides. This means...

  * As with "partial" overrides, the deleted info violates the Rule of One.

  * As with "partial" overrides, the deleted info can be left in multiple topics' info lists.

  * If a "deleted" `INFO` override re-parents the info relative to the info's previously loaded record, then at the time that `TESTopicInfo::LoadForm` (and through it, `TESTopic::OnInfoLoaded`) is called, the info will not be removed from its previous parent, and its cached index will not be reset. This means that if the "deleted" `INFO` override specifies `PNAM` as anything other than zero or `0xFFFFFFFF`, the logic for removing the deleted info (in order to re-insert it at the desired position) will fail: an unrelated info (at the same index as the deleted info's cached index) may be removed from the current topic instead.
  
  This means that it is intrinsically unsafe to allow *both* deleting infos defined outside of the active file, *and* re-parenting infos defined outside of the active file. If a file overrides an info to re-parent it, and then the next-loaded file overrides the same info to delete it, then the info's former parent topic will have its info list mangled. (At the time of this writing, DovahKit does not impose limitations on the deletion or re-parenting of non-active-file infos.)