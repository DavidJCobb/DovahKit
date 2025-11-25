
# Idle tree construction

In order to understand how idle trees are constructed, we must understand a few key facts.

* The basic structure of the loaded idle trees is as follows. The top-level nodes (<dfn>graph nodes</dfn>) represent Havok behavior graphs. A graph node contains a mapping of actions (`AACT` forms) to <dfn>action root</dfn> idles (`IDLE` forms), as well as a list of "loose" idles. Idles can also have child idles. As long as the data is well-formed (we'll get to that), the result can be conceptualized as a tree of nodes: graph nodes contain action nodes and a "loose" node; action nodes contain exactly one idle node; and "loose" nodes and idle nodes can contain an arbitrary number of child idle nodes.
* Idles specify their containing behavior graph via the `DNAM` subrecord, and their parent form and previous sibling via the `ANAM` subrecord. There is a flag in `IDLE/DNAM`, called "parent" by the community, which is intended to force an idle to be loose, skipping the logic that would translate `ANAM` data into a hierarchy placement.
* Bethesda fucked up action root overrides completely, and as a result, degenerate idle trees are possible.

This last point requires some explanation. The vast majority of form types load data from all records, but clear their loaded data at the moment an override (that isn't flagged as "deleted") is encountered, such that the forms only *retain* data from their winning record. This is commonly called the "Rule of One" by the community. Idle trees are walked post-load, after all idle forms are fully loaded, so you might expect the Rule of One to influence an idle's placement in its containing hierarchy. *However*, action roots, very specifically, are handled *at the time that any `ANAM` subrecord is parsed*. Everything else about `ANAM` is based on the winning record, but if the parent form is an action, then that's handled at the time the subrecord is parsed. Worse still, when idles clear their data out upon encountering a non-deleted override record, *they don't un-register themselves as an action root.*[^at-least-they-clear-parentage]

[^at-least-they-clear-parentage]: `TESIdleForm::ClearData` *does* at *least* take the overridden idle out of its parent's child list, if the overridden idle is flagged as "initialized" (read: if it's already been inserted into the idle tree), and it even properly updates the overridden idle's former next-sibling to refer to the former previous-sibling. I'm not quite sure how an override would be loaded *after* the tree is built; maybe it's version control shenanigans. `ClearData` wipes the parent and previous-sibling idle pointers, and the `TESCondition` list; the behavior graph path and plenty of other fields are wiped in `TESIdleForm::InitializeData`, called after `ClearData` when overrides are seen. So everything *except* action roots seems fine, with respect to overrides.

This has a number of implications:

* If a losing record specifies (via `IDLE/ANAM`) an action as the idle's parent, then the idle can potentially be the root for the specified action. It will be in the running for the most recently seen `DNAM` from the same record (i.e. the last `DNAM` from the same record to have loaded before that `ANAM`). This means that if you override an action root idle and try to reparent it, **the idle can end up in multiple places in the overall hierarchy at once**. It can be the action root for multiple actions, and/or it can be the child of another idle, at the same time. Worse, if the idle is transplanted across graphs, then it can end up being an action root in one graph and a loose idle in another graph.
  * The Creation Kit *lets you do this.* It just blithely *lets you* drag-move an action-root idle anywhere you want, and it fails to display the *true effect* of doing so. The idle tree appears to remain well-formed, but if you save and reload the mod, you'll see that the tree is actually degenerate and the idle is now in multiple places at once.
* The same issue can occur if an `IDLE` record is malformed and so contains multiple `ANAM` subrecords.
  * ...or if the file itself is malformed and contains multiple `IDLE` records for the same form, provided any besides the first lack the "deleted" flag.
* If multiple idles try to become the action root for the same graph-and-action at the same time, then the one whose `ANAM` (for that graph-and-action pair specifically) loaded last is the one that wins, displacing the other idle. This appears to be intentional; but if the "losing" idle is in multiple places at once, then its "loss" can hide the idle tree's degeneracy: deleting or moving the "winning" idle may potentially cause the "losing" idle to appear to duplicate (presuming you have something which can accurately display the idle tree in real-time as you make changes).

## Terminology

Because idle trees are potentially degenerate, we need a *lot* of terminology in order to describe their structure and influences.

* **action:** In the context of an idle tree, we are here referring not to an individual action *form*, but to the *pairing* of a behavior graph path and an action form.
* **action root candidacy:** Any `IDLE/ANAM` subrecord occurring in any `IDLE` record (even a losing record) which specifies an action form as the hierarchy parent.
* **winning root [*of an action*]:** Of all idles which have an action root candidacy targeting a given action, the one whose candidacy loaded last is the winning root, and becomes the actual root idle.
  * **active winning root:** The winning root, if the winning candidacy came from the active file.
  * **master winning root:** The winning root, if the winning candidacy came from a non-active file.
* **losing root [*of an action*]:** Any idle that has an action root candidacy targeting the action in question, but whose candidacy loses out to that of another idle.
* **runner-up idle:** If a modification is made to the idle tree (i.e. when viewing and editing the tree in DovahKit) such that an idle ceases to be the winning root of an action, then the <dfn>runner-up idle</dfn> is the idle that becomes the new winning root of that action.
* **canonical graph [*of an idle*]:** The behavior graph path specified by the last-loaded `DNAM` subrecord in the idle's winning record.
* **canonical parent [*of an idle*]:** The idle's "true" location in the idle tree; if the idle is in multiple places at once, this location can be said to be the "main" location.
* **clone [*of an idle*]:** DovahKit has to present the idle tree via a treeview using the Qt library and therefore `QAbstractItemModel`. Qt models (quite reasonably) cannot represent degenerate trees wherein the same node (identified by the same `QModelIndex`) is in multiple places at once, so the UI-side model must create "clone" nodes of idles. You could think of these nodes as akin to "redirect" pages on the web.
  * **clone-of-*Foo*:** Given some idle *Foo*, any clone of that idle is specifically "a clone-of-*Foo*." This notation is used within the tree-editing algorithms described below.

## Algorithms

### For constructing the idle tree

#### overall tree construction

* During file load:
  * When any `IDLE/ANAM` subrecord is seen:
    * Add the idle to a list of <dfn>pending idles</dfn> that need to be incorporated into the global idle tree.
    * Check whether the parent form ID is an action.[^actions-load-before-idles] If so, register the idle as an action root (given the idle's behavior graph and the action in question), and then set the parent form ID to zero: at run-time, the "parent" field on `TESIdleForm` is only ever a `TESIdleForm*`; never a `BGSAction*`.
* Post-load:
  * **Initialize idles and remove duplicates.** Loop over the list of pending idles. If the current list item is already initialized, then remove it; otherwise, initialize it.[^pending-dupes]
  * **For each idle, try inserting it into its parent.**
    * If the idle has the "parent" flag, then add it to the loose idle list for its canonical graph.
    * Otherwise:
      * Let *LooseList* be null.
      * **Validate parentage.** Scan over the idle's parent and ancestors to check for a cyclical reference. If one is found, set this idle's parent and previous-sibling pointers to null, and skip the "Validate siblings" step below.
      * **Validate siblings.** Scan over the idle's previous siblings. If any previous sibling is also an ancestor, then assume that a cyclical reference is formed.[^crusader-kings-are-cyclical] If any of those previous siblings form a cyclical reference, *or* if any sibling has a different parent from this idle, then...
        * Try to set *LooseList* to the loose idle list for an appropriate behavior graph. Specifically, check each of the following idles to see if they have a canonical graph, and use the first such graph you find: this idle; this idle's parent; this idle's previous sibling.
        * Set this idle's parent and previous-sibling pointers to null.
      * **Insert the idle into its parent, if it still has one.** If this idle has a parent, then [perform a sorted insertion](#sorted%20insertion%20of%20an%20idle%20into%20its%20parent) of this idle into its parent.
      * **Insert idle into a loose list, if it has no parent.** If this idle has no parent, *and* is not an action root in its canonical graph, then...
        * If *LooseList* is null, then get-or-create behavior graph info for this idle, and get-or-create a loose idle list for that behavior graph. Set *LooseList* to that idle list.
        * Append this idle to *LooseList* without any sorting.
  * **Recursively crawl all loaded behavior graphs and validate the parentage of all non-loose idles.**
    * If this idle has a parent idle, but does not belong to that parent idle's child list, then this idle is invalid.
    * If this idle has a previous-sibling pointer, but it's its parent's first child *or* the previous idle in its parent's child list is not the same idle as is pointed to by this idle's previous-sibling pointer, then this idle is invalid. Try to silently fix this idle's previous-sibling pointer (forgetting to check if this idle is the first child, so in that case, I think we just corrupt the heap).

[^actions-load-before-idles]: This check only works because the game and CK always load the `AACT` record group before the `IDLE` record group. All Action forms (`BGSAction`) are guaranteed to be loaded by the time the game processes any idle's `ANAM` subrecord. Therefore the game can take the parent record ID listed in `IDLE/ANAM`, convert it to a form ID, do a form lookup early, and check if the result exists and is an Action form.

[^pending-dupes]: If an `IDLE` record is overridden, then multiple pointers to that idle will end up in the pending idles list. This is because the game doesn't skip overridden records; instead, it just manually clears a form's data between files. As such, each `IDLE` record's `ANAM` subrecord is seen, and causes the idle to be added to the pending idles list.

[^crusader-kings-are-cyclical]: Bethesda uses [their equivalent of] a `std::set<TESIdleForm*>` to detect cyclical references. They reuse the same set for parents and previous siblings.

#### sorted insertion of an idle into its parent

Bethesda's engine performs sorted insertions on idles (if their parent is another idle), rather than sorting them after the fact. The 32-bit build of the Creation Kit features both a recursive implementation (seen on `BGSCameraPath`, which uses similar tree construction) and a non-recursive implementation (seen on `TESIdleForm`), with the latter presumably being the result of inlining by the compiler.

##### Recursive implementation

* Let *CurrentIdle* be the idle to insert.
* Search the destination list for *CurrentIdle*'s previous sibling. If it *has* a desired previous sibling, and if that sibling is in the destination list, then insert *CurrentIdle* after it; else, append *CurrentIdle* to the end of the list.
* **Re-sort all next siblings of *CurrentIdle*.**
  * Let *CurrentIndex* be -1.
  * Let *NextIndex* be -1. Let *NextIdle* be null.
  * Search the parent's child list for *CurrentIdle*, and for an idle that identifies *CurrentIdle* as its previous sibling. Store the index of both idles as *CurrentIndex* and *NextIndex*, respectively, and set *NextIdle* to the latter idle.
    * If multiple idles identify *CurrentIdle* as their previous sibling, then (if you're the CK) emit a warning, and only pay attention to the first such idle.
  * If *NextIndex* is -1, then there is no next idle; exit.
  * If *NextIndex* = *CurrentIndex* + 1, then the next idle is already in its proper place; exit.
  * Take the idle at *NextIndex*, and move it within the list to *CurrentIndex* + 1.
  * **Re-sort all next siblings of *NextIdle*.**

##### Non-recursive implementation

* Let *CurrentIdle* be the idle to insert.
* Search the destination list for *CurrentIdle*'s previous sibling. If it *has* a desired previous sibling, and if that sibling is in the destination list, then insert *CurrentIdle* after it; else, append *CurrentIdle* to the end of the list.
* Loop indefinitely:
  * Let *NextIndex* be -1. Let *NextIdle* be null.
  * Search the parent's child list for *CurrentIdle*, and for an idle that identifies *CurrentIdle* as its previous sibling. Store the index of both idles as *CurrentIndex* and *NextIndex*, respectively, and set *NextIdle* to the latter idle.
    * If multiple idles identify *CurrentIdle* as their previous sibling, then (if you're the CK) emit a warning, and only pay attention to the first such idle.
  * If *NextIndex* is -1, then there is no next idle; break out of the loop.
  * If *NextIndex* = *CurrentIndex* + 1, then the next idle is already in its proper place; break out of the loop.
  * Take the idle at *NextIndex*, and move it within the list to *CurrentIndex* + 1.
  * Set *CurrentIdle* to *NextIdle*.

See [Appendix A](#Appendix%20A) for further notes and an example of the worst-case complexity.

### For describing the idle tree

#### computing an idle's canonical location

This is, in essence, an abbreviated description of the algorithm for constructing the overall idle tree:

* Let *IsLoose* be *false*.
* Let *LooseIdleParent* be the loose idle container for this idle's canonical graph.
* If the idle has flag 1 set in the winning record's `DATA` subrecord, then set *IsLoose* to *true*.
* Else:
  * If any of the following conditions are met, then set *IsLoose* to *true*:
    * The winning `ANAM` subrecord specifies no parent form, or a parent form that is not an `IDLE`.
    * The winning `ANAM` subrecord specifies an invalid hierarchy position (i.e. cyclical parentage, cyclical siblinghood, any sibling is also an ancestor, or any previous sibling has an inconsistent parent).
      * If the problem is any issue besides cyclical parentage, and if *LooseIdleParent* is null (i.e. because the subject idle specifies no canonical graph), then set *LooseIdleParent* to the loose idle container for the parent's canonical graph (if the parent exists and has a canonical graph), or otherwise to the previous sibling's canonical graph (if the previous sibling exists and has a canonical graph).
* If *IsLoose*:
  * If the idle is the winning root of any action in its canonical graph, then out of all those actions, return the action with the last-loaded winning candidacy.
  * Else, return *LooseIdleParent*.
* Return the parent idle specified in `ANAM`.

Therefore after the idle tree is constructed, the canonical parent of an idle can, in general, be identified as follows:

* If the idle has a parent idle, then that's the canonical parent.
* If the idle has any action root candidacies belonging to its canonical graph, then the last-loaded of those candidacies indicates the action that is the idle's canonical parent.
* Otherwise, the idle *must* (given how idle trees are constructed) be a loose idle within some graph (which may or may not be its canonical graph; see above), and so that graph's loose idle container is the canonical parent.

### Modifying the idle tree

DovahKit attempts to represent the idle tree as it would exist in-game. Because both the Creation Kit and community tools like xEdit allow for the construction of degenerate idle trees, we have to properly represent those trees &mdash; ideally in real-time. This greatly complicates even relatively simple tasks such as moving an idle from one place to another.

Additionally, as noted in the terminology section (much) further above, DovahKit has to present idles in a treeview via `QAbstractItemModel`, which (quite reasonably) is incapable of representing degenerate data hierarchies. This means that you'll see reference to "clones." The datastore never creates clones, but will have to expose callbacks sufficient for a `QAbstractItemModel` to create and manage clones.

#### moving an idle

This algorithm defines the process of moving *SubjectIdle* to *Destination*, as a modification performed upon *SubjectIdle* after the datastore has been built. The symbol 🛂 is used to prefix model-related operations (particularly those for which external before-and-after callbacks must fire). For example, "🛂moving" a node means *just* transplanting the node from one place to another, without recursively executing this entire algorithm.

* **If moving to an action, displace any action root which is already there.** If *Destination* is an action node, then:
  * Let *Graph* be the containing graph node for *Destination*.
  * Let *DisplacedIdle* be the current winning root for *Destination*, if any.
  * If *DisplacedIdle* exists, then:
    * If *DisplacedIdle*'s canonical parent node is *Destination*:
      * Let *DisplacedFromActive* be *true* if *DisplacedIdle* is the active winning root for *Destination*, or *false* otherwise.
      * If *DisplacedFromActive*, then:
        * Recursively execute this algorithm to move *DisplacedIdle* to *Graph*'s loose idle container. (Recursive execution is necessary in case *DisplacedIdle* is the active winning root of multiple actions.)
      * Else:
        * 🛂Move *DisplacedIdle* to *Graph*'s loose idle container.
        * 🛂Edit *DisplacedIdle*'s form data to represent its new hierarchy position.
    * Else if a clone-of-*DisplacedIdle* exists at *Destination*:
      * 🛂Delete the clone-of-*DisplacedIdle*.
* **Update the subject's hierarchy placement.**
  * Let *SubjectHasBeenMoved* be *false*.
  * Let *DroppedFromActions* be the list of actions for which *SubjectIdle* has any active-file action root candidacies.
  * Let *DisqualifiedFromActions* be those actions from *DroppedFromActions* which have *SubjectIdle* as their active winning root.
  * **Destroy all of the subject's active-file action root candidacies, save for that of the canonical parent.**  For each action *Action* of *DroppedFromActions*:
    * If *Action* is not *SubjectIdle*'s canonical parent, then:
      * If *Action* contains a clone-of-*SubjectIdle* node:
        * 🛂Delete that clone-of-*SubjectIdle* node.
        * For *Action*, [handle runner-ups after loss of an action root](#handling%20runner-ups%20after%20loss%20of%20an%20action%20root).
      * Delete all active-file action root candidacies, tracked by *Action*, that pertain to *SubjectIdle*.
  * Let *FormerNextSibling* be null.
  * **Move the subject.**
    * Let *MovedFrom* be *SubjectIdle*'s canonical parent.
    * If *MovedFrom* is an idle:
      * Set *FormerNextSibling* to *SubjectIdle*'s next sibling within *MovedFrom*.
    * If *MovedFrom* is an action:
      *  Delete all active-file action root candidacies, tracked by *MovedFrom*, that pertain to *SubjectIdle*.
    * 🛂Move *SubjectIdle* from its canonical parent to *Destination*.
      * This should also sever *SubjectIdle*'s outbound active-file action root candidacies, and (if *Destination* is an action) create a new such candidacy as appropriate.
    * **Update form hierarchy data.**
      * 🛂Edit *SubjectIdle*'s form data to represent its new hierarchy position.
      * If *FormerNextSibling* is not null, then 🛂edit its form data to point to its new previous sibling.
      * If *Destination* is an idle:
        * Let *NewNextSibling* be *SubjectIdle*'s previous sibling after having been moved.
        * If *NewNextSibling* is not null, then 🛂edit its form data to point to *SubjectIdle* as its new previous sibling.
    * **Update the action that the subject was moved from.** If *MovedFrom* is an action:
      * If *SubjectIdle* is still the winning root of *MovedFrom*:[^still-winning-after-move]
        * 🛂Clone *SubjectIdle* into *MovedFrom*.[^move-and-then-clone-back-home]
      * Else:
        * For *MovedFrom*, [handle runner-ups after loss of an action root](#handling%20runner-ups%20after%20loss%20of%20an%20action%20root).

[^move-and-then-clone-back-home]: The effect of this operation is that *SubjectIdle* ends up in two places at once, and "isn't moved" out of *MovedFrom*, but the instance of the subject in *Destination* is the canonical one and the instance in *MovedFrom* is a clone.

[^still-winning-after-move]: This can happen if *SubjectIdle* is the winning root of *MovedFrom* due both to an active-file action root candidacy *and* a non-active-file action root candidacy.

Consequently the potential sequence of callbacks is:

* *displacing a prior root*
  * Idle moved (displaced idle)
  * Form data edited (displaced idle)
  * Idle is no longer multiply present at a particular root (displaced idle)
* *updating the subject's hierarchy placement*
  * *destroying non-canonical active-file action root candidacies*
    * Idle is no longer multiply present at a particular root (subject idle)
    * *handling runner-ups after loss of an action root:*
      * *in any order, zero or more times:*
        * Move a different (runner-up) idle
        * Clone a different (runner-up) idle
  * *moving the subject*
    * Idle moved (subject idle)
    * *updating form hierarchy data*
      * Form data edited (subject idle)
      * Form data edited (former next-sibling)
      * Form data edited (new next-sibling)
    * *updating the action the subject was moved from*
      * *either of:*
        * Idle is now multiply present at a particular root (subject idle)
        * *handling runner-ups after loss of an action root:*
          * *in any order, zero or more times:*
            * Move a different (runner-up) idle
            * Clone a different (runner-up) idle

and algorithm-based movement of the intended idle may be deferred until after algorithm-based movement of a displaced idle.

#### deleting an idle from the active file

This algorithm defines the process of deleting *SubjectIdle* from the active file. (If an idle is defined in one of the active file's masters, then deleting it just creates an active file override with the "deleted" flag, which has no effect on its hierarchy placement.)

* Let *Actions* be the list of actions for which *SubjectIdle* has any action root candidacies.
* Assert that *SubjectIdle* is not a master winning root for any action in *Actions*.
* **Delete extant clones of the subject first.** For each action *Action* in *Actions*:
  * If *Action* is not *SubjectIdle*'s canonical parent:
    * If *Action* contains a clone-of-*SubjectIdle* node:
      * 🛂Delete that clone-of-SubjectIdle node.
      * Delete all active-file action root candidacies, tracked by *Action*, that pertain to *SubjectIdle*.
* **Delete the subject.**
  * If *SubjectIdle*'s canonical parent is an action:
    * Delete all active-file action root candidacies, tracked by that canonical parent, that pertain to *SubjectIdle*.
  * 🛂Delete *SubjectIdle*. If *SubjectIdle* was the child of another idle, then update form data for its former previous- and next-siblings.
* For *Actions*, [handle runner-ups after loss of an action root](#handling%20runner-ups%20after%20loss%20of%20an%20action%20root).

Consequently the potential sequence of callbacks is:

* Delete zero or more clones of the subject idle
* Delete the subject idle
* *zero, one, or two times:*
  * Edit a different (formerly adjacent sibling) idle's form data
* *handling runner-ups after loss of an action root:*
  * *in any order, zero or more times:*
    * Move a different (runner-up) idle
    * Clone a different (runner-up) idle

#### handling runner-ups after loss of an action root

This algorithm, performed on a list of *SubjectActions*, may need to be invoked by the other algorithms for more fundamental operations such as moving or deleting an idle.

* **Handle runner-ups for any of the subject's former active-file action root candidacies.** For each action *Action* of *SubjectActions*:
  * Let *Graph* be the containing graph of *Action*.
  * Let *RunnerUpIdle* be *Action*'s new winning root.
  * If *RunnerUpIdle*'s canonical parent node is already *Action*, then exit.
  * Else if *RunnerUpIdle*'s canonical parent node is the loose idle container of *Graph*:
    * 🛂Move *RunnerUpIdle* from the loose idle container of *Graph*, to *Action*.
  * Else:
    * 🛂Clone *RunnerUpIdle* into *Action*. (It must be the case that *RunnerUpIdle*'s canonical parent is either some other action root, another idle, or the loose idle container of a different graph.)

## Interface

For moving an idle, I think we'll want to divide the steps into these functions, each passkeyed as appropriate:

* `void action_node::_displace_current_winning_root();`
  * Represents the entire "displace whatever root is already there" part of the algorithm.
* `void action_node::_idle_is_no_longer_an_active_candidate(const idle_node&);`
  * Invoked when looping over *SubjectIdle*'s active-file candidacies (except that of the canonical parent). If the `idle_node`'s parent node isn't the `action_node`, then this invokes the "idle is no longer multiply present in a particular action" callback.
* `void idle_node::_replace_active_file_candidacies_with(action_node*);`
* `void action_node::_on_winning_root_changed();`
  * Represents the "handle runner-ups after loss of an action root" algorithm as invoked for a single action.

These functions should be useful for deleting idles as well.

# Appendix A

When I first discovered the non-recursive version of the sorted insertion algorithm for idles and camera paths, I initially struggled a bit to grasp it. My notes follow:

So why are we moving potentially multiple idles? Why the loop? Consider the case of five idles whose desired order is {A, B, C, D, E}, but which are loaded in the order {A, D, C, B, E}.

<table>
   <tr>
      <th>New element</th>
      <th>List</th>
      <th>Note</th>
   </tr>
   <tr>
      <th rowSpan="2">A</th>
      <td>A</td>
      <td>First insertion.</td>
   </tr>
   <tr>
      <td>A</td>
      <td>A's next sibling is not present in the list.</td>
   </tr>
   <tr>
      <th rowSpan="2">D</th>
      <td>A, D</td>
      <td>Previous sibling C is not yet present.</td>
   </tr>
   <tr>
      <td>A, D</td>
      <td>D's next sibling is not present in the list.</td>
   </tr>
   <tr>
      <th rowSpan="3">C</th>
      <td>A, D, C</td>
      <td>Previous sibling B is not yet present.</td>
   </tr>
   <tr>
      <td>A, C, D</td>
      <td>C's next sibling D is present, and is re-sorted.</td>
   </tr>
   <tr>
      <td>A, C, D</td>
      <td>D's next sibling is not present in the list.</td>
   </tr>
   <tr>
      <th rowSpan="3">B</th>
      <td>A, B, D, C</td>
      <td>Previous sibling A is present.</td>
   </tr>
   <tr>
      <td>A, B, C, D</td>
      <td>B's next sibling C is present, and is re-sorted.</td>
   </tr>
   <tr>
      <td>A, B, C, D</td>
      <td>D's next sibling is not present in the list.</td>
   </tr>
   <tr>
      <th rowSpan="2">E</th>
      <td>A, B, C, D, E</td>
      <td>Previous sibling D is present.</td>
   </tr>
   <tr>
      <td>A, B, C, D, E</td>
      <td>E has no next sibling.</td>
   </tr>
</table>

A more pathological case would look like this:

<table>
   <tr>
      <th>New element</th>
      <th>List</th>
      <th>Step</th>
      <th>Note</th>
   </tr>
   <tr>
      <th rowSpan="2">E</th>
      <td>E</td>
      <td>Insert</td>
      <td>First insertion.</td>
   </tr>
   <tr>
      <td>E</td>
      <td>Sort</td>
      <td>E has no next sibling.</td>
   </tr>
   <tr>
      <th rowSpan="3">D</th>
      <td>E, D</td>
      <td>Insert</td>
      <td>Previous sibling C is not yet present.</td>
   </tr>
   <tr>
      <td>D, E</td>
      <td>Sort</td>
      <td>D's next sibling is E, and is moved.</td>
   </tr>
   <tr>
      <td>D, E</td>
      <td>Sort</td>
      <td>E has no next sibling.</td>
   </tr>
   <tr>
      <th rowSpan="4">C</th>
      <td>D, E, C</td>
      <td>Insert</td>
      <td>Previous sibling B is not yet present.</td>
   </tr>
   <tr>
      <td>E, C, D</td>
      <td>Sort</td>
      <td>C's next sibling is D, and is moved.</td>
   </tr>
   <tr>
      <td>C, D, E</td>
      <td>Sort</td>
      <td>D's next sibling is E, and is moved.</td>
   </tr>
   <tr>
      <td>C, D, E</td>
      <td>Sort</td>
      <td>E has no next sibling.</td>
   </tr>
   <tr>
      <th rowSpan="5">B</th>
      <td>C, D, E, B</td>
      <td>Insert</td>
      <td>Previous sibling B is not yet present.</td>
   </tr>
   <tr>
      <td>D, E, B, C</td>
      <td>Sort</td>
      <td>B's next sibling is C, and is moved.</td>
   </tr>
   <tr>
      <td>E, B, C, D</td>
      <td>Sort</td>
      <td>C's next sibling is D, and is moved.</td>
   </tr>
   <tr>
      <td>B, C, D, E</td>
      <td>Sort</td>
      <td>D's next sibling is E, and is moved.</td>
   </tr>
   <tr>
      <td>B, C, D, E</td>
      <td>Sort</td>
      <td>E has no next sibling.</td>
   </tr>
   <tr>
      <th rowSpan="6">A</th>
      <td>B, C, D, E, A</td>
      <td>Insert</td>
      <td>A has no previous sibling.</td>
   </tr>
   <tr>
      <td>C, D, E, A, B</td>
      <td>Sort</td>
      <td>A's next sibling is B, and is moved.</td>
   </tr>
   <tr>
      <td>D, E, A, B, C</td>
      <td>Sort</td>
      <td>B's next sibling is C, and is moved.</td>
   </tr>
   <tr>
      <td>E, A, B, C, D</td>
      <td>Sort</td>
      <td>C's next sibling is D, and is moved.</td>
   </tr>
   <tr>
      <td>A, B, C, D, E</td>
      <td>Sort</td>
      <td>D's next sibling is E, and is moved.</td>
   </tr>
   <tr>
      <td>A, B, C, D, E</td>
      <td>Sort</td>
      <td>E has no next sibling.</td>
   </tr>
</table>

This is a doubly-nested loop: one outer loop iteration per element to sort after insertion; and one inner loop iteration per element already present. (The inner loop doesn't early-out, at least in the CK, as that would prevent it from error-checking the case of multiple idles wanting the same previous sibling.) This means that in the worst case, the total iteration count for sorting is $`\sum_{i=1}^{n}n`$.

For 5 children, the worst-case sort iteration count is 55.