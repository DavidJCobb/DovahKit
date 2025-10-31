
# Idle tree construction

At run-time, all animations are organized into a tree. The top-level nodes are behavior graphs; those contain idles (`TESIdleForm`) that are either action roots or loose idles. Idles, in turn, can contain a tree of other idles. An <dfn>action root</dfn> is the root idle associated with a single actor action (`BGSAction`) within a behavior graph, whereas a <dfn>loose idle</dfn> isn't the root idle for an action but also isn't a loose idle.

When serialized into a data file, idles store:

* The form ID of their parent action (if the idle is an action root) or their parent idle.
* The form ID of their previous-sibling idle, if any.

As such, after all forms have been loaded, the game must build an idle tree. The overall process used by the game and CK is this:

* During file load:
  * When any `IDLE/ANAM` subrecord is seen:
    * Add the idle to a list of <dfn>pending idles</dfn> that need to be incorporated into the global idle tree.
    * Check whether the parent form ID is an action. If so, register the idle as an action root, and then set the parent form ID to zero: at run-time, the "parent" field on `TESIdleForm` is only ever a `TESIdleForm*`.
* Post-load:
  * **Initialize idles and remove duplicates.** Loop over the list of pending idles. If the current list item is already initialized, then remove it; otherwise, initialize it.[^pending-dupes]
  * **For each idle, try inserting it into its parent.**
    * If the idle has the "parent" flag, then add it to the loose idle list for its behavior graph.
    * Otherwise:
      * Let *LooseList* be null.
      * **Validate parentage.** Scan over the idle's parent and ancestors to check for a cyclical reference. If one is found, set this idle's parent and previous-sibling pointers to null, and skip the "Validate siblings" step below.
      * **Validate siblings.** Scan over the idle's previous siblings. If any previous sibling is also an ancestor, then assume that a cyclical reference is formed.[^crusader-kings-are-cyclical] If any of those previous siblings form a cyclical reference, *or* if any sibling has a different parent from this idle, then...
        * Try to set *LooseList* to the loose idle list for an appropriate behavior graph. Specifically, check each of the following idles to see if they have a containing behavior graph, and use the first such graph you find: this idle; this idle's parent; this idle's previous sibling.
        * Set this idle's parent and previous-sibling pointers to null.
      * **Insert idle into its parent, if it still has one.** If this idle has a parent, then...
        * Append this idle into its parent's child list.
        * Iteratively re-sort this idle's next siblings within that child list.
        * Done.
      * **Insert idle into a loose list, if it has no parent.** If this idle has no parent, *and* is not an action root, then...
        * If *LooseList* is null, then get-or-create behavior graph info for this idle, and get-or-create a loose idle list for that behavior graph. Set *LooseList* to that idle list.
        * Append this idle to *LooseList*.
  * **Recursively crawl all loaded behavior graphs and validate the parentage of all non-loose idles.**
    * If this idle has a parent idle, but does not belong to that parent idle's child list, then this idle is invalid.
    * If this idle has a previous-sibling pointer, but it's its parent's first child *or* the previous idle in its parent's child list is not the same idle as is pointed to by this idle's previous-sibling pointer, then this idle is invalid. Try to silently fix this idle's previous-sibling pointer (forgetting to check if this idle is the first child, so in that case, I think we just corrupt the heap).

[^pending-dupes]: If an `IDLE` record is overridden, then multiple pointers to that idle will end up in the pending idles list. This is because the game doesn't skip overridden records; instead, it just manually clears a form's data between files. As such, each `IDLE` record's `ANAM` subrecord is seen, and causes the idle to be added to the pending idles list.

[^crusader-kings-are-cyclical]: Bethesda uses [their equivalent of] a `std::set<TESIdleForm*>` to detect cyclical references. They reuse the same set for parents and previous siblings.

## Potential invalid data and mitigations

These are the mitigations employed by the Creation Kit while building the idle tree.

| Problem | Mitigation |
| :- | :- |
| An idle is its own parent or previous sibling. | This is a cyclical reference of length 1 and trips one of those cases below. |
| An idle has no parent and no previous sibling. | The idle is appended to its behavior graph's loose idle list. |
| A group of sibling idles each have no parent. | ??? |
| A group of idles has a cyclical parent idle chain. | The first-seen idle in the chain is made a loose idle[^mitigate-make-loose], breaking the chain. |
| An idle has an inconsistent parent idle from any of its siblings. | Each of the siblings is made a loose idle[^mitigate-make-loose] as they are processed.[^mitigate-siblings-individually] |
| A group of idles has a cyclical sibling chain. | Each of the siblings is made a loose idle[^mitigate-make-loose] as they are processed.[^mitigate-siblings-individually] |
| Multiple idles in the same behavior graph have the same parent action. | ??? |
| A group of sibling idles have different parent actions. | ??? |
| An idle is initially defined as having a parent action, but its parent is changed in an override record. | No mitigation. The idle may become the root for all of the parent actions specified across the base record and overrides. |

[^mitigate-make-loose]: The idle's parent and previous-sibling pointers are set to null before the idle is added to any idle tree.

[^mitigate-siblings-individually]: Making an idle loose, by nulling its parent, will cause it to have an inconsistent (null) parent from its next sibling(s), such that they test as having inconsistent parents and are themselves made loose.

These are the mitigations employed by the Creation Kit when validating the idle tree afterward.

| Problem | Mitigation | Note |
| :- | :- | :- |
| *X* has a parent idle, but *X* was added to some other list of siblings. | No mitigation. | This may be possible for action roots depending on how overrides are handled. It is impossible for idles that are children of other idles. |
| *X* is, somehow, not in the sibling list that it is being validated against. | Warning dialog ("Parent array does not contain *idle*"). | This should be flat-out impossible. |
| *X* has a previous sibling pointer, but its actual previous sibling is a different idle or no idle. | Warning dialog ("Invalid prev idle on *idle*"). The game attempts to correct *X*'s previous-sibling but may fail to do so reliably. | This could happen if a mod reorders idles exclusively by modifying the single idle being moved.[^unsafe-reordering] |

[^unsafe-reordering]: The intended way to reorder or reparent an idle is by overriding the moved idle, the idle after the destination position (if the idle isn't being moved to the end), and the idle after the source position (if the idle isn't being moved from the end).
  
  If you reparent an idle but only override that idle, then its former next sibling will still identify it as a previous sibling, leading to invalid data wherein apparent siblings have inconsistent parents.
  
  If you reorder an idle within its parent to place it after *Y*, but you only override the moved idle, then the ordering of the moved idle with respect to its former and new next-siblings is undefined (i.e. the final order of the loaded idles depends on what order the game ends up processing the forms in). Similar issues can be expected if you only override the former next sibling but not the new next sibling, or vice versa.

## Sorting the idles

Bethesda's engine performs sorted insertions on idles. The algorithm is as follows:

* Let *CurrentIdle* be the idle to insert.
* Search the destination list for *CurrentIdle*'s previous sibling. If found, insert *CurrentIdle* after it; else, append *CurrentIdle* to the end of the list.
* Loop indefinitely:
  * Let *NextIndex* be -1. Let *NextIdle* be null.
  * Search the parent's child list for *CurrentIdle*, and for an idle that identifies *CurrentIdle* as its previous sibling. Store the index of both idles as *CurrentIndex* and *NextIndex*, respectively, and set *NextIdle* to the latter idle.
    * If multiple idles identify *CurrentIdle* as their previous sibling, then (if you're the CK) emit a warning, and only pay attention to the first such idle.
  * If *NextIndex* is -1, then there is no next idle; break out of the loop.
  * If *NextIndex* = *CurrentIndex* + 1, then the next idle is already in its proper place; break out of the loop.
  * Take the idle at *NextIndex*, and move it within the list to *CurrentIndex* + 1.
  * Set *CurrentIdle* to *NextIdle*.

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

### What if we sorted after all insertions?

The basic algorithm for that would be:

* Let *N* be the number of children to sort.
* Let *CurrentIndex* be 0.
* Loop indefinitely:
  * Let *CurrentIdle* be the idle at position *CurrentIndex*.
  * Let *PreviousIndex* be the index of the idle that *CurrentIdle* identifies as its previous sibling, or -1 if no such idle is present.
  * If *PreviousIndex* is -1:
    * Increment *CurrentIndex*.
    * Skip to the next loop iteration.
  * Move *CurrentIdle* after *PreviousIndex*.

An example of the worst-case sort:

* E, D, C, B, A
* D, E, C, B, A
* E, C, D, B, A
* C, D, E, B, A
* D, E, B, C, A
* E, B, C, D, A
* B, C, D, E, A
* C, D, E, A, B
* D, E, A, B, C
* E, A, B, C, D
* A, B, C, D, E

If we assume that we always loop over the full list to find *PreviousIndex*, then this is a nested loop, and our worst-case total iteration count is $`\sum_{i=1}^{n - 1}n`$. However, the only reason not to bail out of the inner loop early is for data validation, and we can't validate that a single idle has multiple next idles (i.e. that two or more idles share the same previous sibling).

For 5 children, the worst-case iteration count is 50.