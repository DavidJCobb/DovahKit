
# `QuestAllDialogueModel`

The intended design for this model is as follows:

* A quest-editing dialogue creates a single such model and points the model at the quest.
* The model processes all dialogue associated with the quest and caches data as usual.
* The quest-editing dialog creates proxy models for each listview used for editing dialogue.
  * There are six categories of dialogue, with all except scene-associated dialogue being edited using the same basic UI, consisting of three tableviews (branches, topics, and infos). The quest-editing dialog would have a tabbox for picking between these, and in each tab the dialog would instantiate a separate copy of that UI. Each tableview would be given its own proxy model that sources from the core "all dialogue for this quest ever" model.

More generally, the intended UI design is:

* Editing properties of a branch or a topic requires opening a child dialog. This way, you can OK/Cancel your changes, instead of those changes being written directly to the form in real-time even if you cancel the Quest. Having OK/Cancel on the branches and topics themselves will (hopefully) make it (slightly) clear(er) when you're actually committing changes to something.
* The CK uses separate top-level tabs for editing various kinds of dialogue that share a UI. We instead use a nested tabbox.
* Only Player Dialogue uses branches. For all other categories, the branch tableview should be greyed out, and the topic list should use `QuestBranchlessTopicsModel` (filtering to the appropriate dialogue subtypes).