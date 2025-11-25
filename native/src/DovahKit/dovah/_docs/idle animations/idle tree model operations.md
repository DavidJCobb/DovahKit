
## Algorithms

### Moving an idle

This algorithm defines the process of moving *SubjectIdle* to *Destination*, as a modification performed upon *SubjectIdle* after the datastore has been built. The symbol 🛂 is used to prefix model-related operations (particularly those for which external before-and-after callbacks must fire). For example, "🛂moving" a node means *just* transplanting the node from one place to another, without recursively executing this entire algorithm.

* **If moving to an action, displace any action root which is already there.** If *Destination* is an action node, then:
  * Let *Graph* be the containing graph node for *Destination*.
  * Let *DisplacedIdle* be the current winning root for *Destination*, if any.
  * If *DisplacedIdle* exists, then:
    * If *DisplacedIdle*'s canonical parent node is *Destination*:
      * Let *DisplacedFromActive* be *true* if *DisplacedIdle* is the active winning root for *Destination*, or *false* otherwise.
      * 🛂Move *DisplacedIdle* to *Graph*'s loose idle container.
      * If *DisplacedFromActive*, then 🛂edit *DisplacedIdle*'s form data to represent its new hierarchy position.
    * Else if a clone-of-*DisplacedIdle* exists at *Destination*:
      * 🛂Delete the clone-of-*DisplacedIdle*.
* **Update the subject's hierarchy placement.**
  * Let *SubjectHasBeenMoved* be *false*.
  * Let *DroppedFromActions* be the list of actions for which *SubjectIdle* has any active-file action root candidacies.
  * Let *DisqualifiedFromActions* be those actions from *DroppedFromActions* which have *SubjectIdle* as their active winning root.
  * **Destroy the subject's active-file action root candidacies.** For each action *Action* of *DroppedFromActions*:
    * If *Action* is *SubjectIdle*'s canonical parent, then:
      * 🛂Move *SubjectIdle* from *Action* to *Destination*.
      * Set *SubjectHasBeenMoved* to *true*.
    * Else:
      * If *Action* contains a clone-of-*SubjectIdle* node:
        * 🛂Delete that clone-of-*SubjectIdle* node.
    * Delete all active-file action root candidacies, tracked by *Action*, that pertain to *SubjectIdle*.
  * If *SubjectHasBeenMoved* is *false*, then...[^subject-fallback-move]
    * Let *MovedFrom* be *SubjectIdle*'s canonical parent.
    * 🛂Move *SubjectIdle* from its canonical parent to *Destination*.
    * If *MovedFrom* is an action, then... (By implication, *Subject* must be a master winning root for that action.)
      * 🛂Clone *SubjectIdle* into *MovedFrom*.
    * Set *SubjectHasBeenMoved* to *true*.
  * 🛂Edit *SubjectIdle*'s form data to represent its new hierarchy position.
  * **Handle runner-ups for any of the subject's former active-file action root candidacies.** For each action *Action* of *DisqualifiedFromActions*:
    * Let *RunnerUpIdle* be *Action*'s new winning root.
    * If *RunnerUpIdle*'s canonical parent node is the loose idle container of *Graph*:
      * 🛂Move *RunnerUpidle* from the loose idle container of *Graph*, to *Action*.
    * Else:
      * 🛂Clone *RunnerUpIdle* into *Action*. (It must be the case that *RunnerUpIdle*'s canonical parent is either some other action root, another idle, or the loose idle container of a different graph.)

[^subject-fallback-move]: This occurs if the subject: isn't an active winning root; or is an active winning root, but is also canonically located elsewhere due to having a malformed `IDLE` record (i.e. multiple `ANAM` subrecords in a single record, or multiple records for the same form in the same file) such that its last-loaded winning-record `ANAM` places it inside of an idle.

## Deleting an idle