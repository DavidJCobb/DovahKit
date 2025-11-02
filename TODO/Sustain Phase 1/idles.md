
# Idles

Idles are... tricky. You can find a full overview [here](../../native/src/DovahKit/dovah/_docs/idle%20animations/idle%20tree%20construction.md), but the gist of it is that they're organized into a three-plus-level tree: graphs -> actions -> idles, with idles being able to contain each other as well. Within the game's file format, an `IDLE` form specifies its containing graph, its parent action or idle, and its previous-sibling idle. When initially building the tree, it's relatively straightforward to handle invalid idle edits the same way the game does (or, in some cases, the same way it *doesn't*). However, maintaining those invariants *after* building the tree is harder.

The gist is this. Suppose we have the following tree:

* :large_red_square: /Actors/MyCoolBehaviorGraph.hkx
  * :large_blue_square: ActionIdle
    * :large_white_square: MyCoolIdleRoot
      * :large_white_square: MyCoolIdleA
      * :large_white_square: MyCoolIdleB
  * :large_blue_square: ActionIdleStop
    * :large_white_square: MyCoolIdleStopRoot
      * :large_white_square: MyCoolIdleStop
      * :large_white_square: MyCoolIdleStopFallback

In DovahKit, this tree is currently maintained by a "datastore" object. This exists in the liminal space between the backend and the frontend. It's not part of the backend: it doesn't influence how we handle basic operations on forms. It's provided as a utility to the frontend, to the UI: it creates a tree of nodes that describe the tree above, with behavior graph nodes wrapping a `std::string` and action and idle nodes containing a reference to a form stub.

If I want to move `MyCoolIdleStop` between `MyCoolIdleA` and `MyCoolIdleB`, the *correct* sequence of edits to make is as follows:

* Edit `MyCoolIdleStopFallback`: set the previous sibling to `NONE`.
* Edit `MyCoolIdleStop`:
  * Set the parent to `MyCoolIdleRoot`.
  * Set the previous sibling to `MyCoolIdleA`.
* Edit `MyCoolIdleB`: set the previous sibling to `MyCoolIdleStop`.

However, there's absolutely nothing stopping anything outside the frontend from just... *not* performing all of those steps. The result would be an invalid idle tree, with potentially wide-ranging effects. A few examples based on static analysis of the 32-bit Creation Kit:

* If we change the previous sibling of `MyCoolIdleStopFallback` but not its parent, and otherwise perform all of the steps listed above, then the group of siblings within `MyCoolIdleRoot` will no longer have a consistent parent. When loading game data, this would cause *all* of them to become loose idles.
* If we only edit `MyCoolIdleStopFallback` and no other forms, then `MyCoolIdleStopFallback` will have a different parent from its previous sibling. The CK will correct this by making `MyCoolIdleStopFallback` a loose idle.

Skip any of those steps, and a variety of invalid hierarchies could result, and some of them *may* have undefined behavior i.e. the exact effects *could* depend on the order in which idle forms are processed post-load. ("May" and "could" here mean "it's unclear due to some of the weirdness in how invalid data is handled," as opposed to "it can happen conditionally.") There are other error conditions that the Creation Kit doesn't even appear to check and warn for, such as reparenting an "action root" idle from one action to another, or giving the same action multiple root idles.

And the trick, then, is this: suppose we have a datastore, a computed idle tree, in memory. Suppose something external to that tree modifies an idle's relationships in an invalid way. Replicating the resulting error conditions within the tree &mdash; knowing how to butcher the existing tree &mdash; is very difficult. In practice, DovahKit as it currently exists offers no handling for this: the datastore doesn't have an `update_idle` member function that responds to arbitrary changes to idles. If the frontend detects that something capable of producing invalid idle edits (e.g. Dovahscript) may have run, it has no choice but to blow away the entire datastore and rebuild it from scratch.

## How do we solve this problem?

We don't solve it. We avoid it.

## How do we avoid this problem?

The process of computing and maintaining an idle tree needs to be built into the backend. It shouldn't be a datastore provided for use by the frontend; it should be an integral system within the backend.

* Idles should store their "desired parent" and "desired previous sibling" pointers, as private/passkeyed state during the load process.
* Post-load, we should build an idle tree as a core part of the backend, owned by the `file_load_order`. The "desired" pointers should be used to construct the tree, and then immediately cleared out. It should no longer be possible to edit them independently.
* All changes to the idle hierarchy should go through the tree.
* When an `IDLE` is being saved, we should write its data based on what parent and previous sibling the tree says it should have.
* If anything external to the frontend needs to know what an idle's desired parent and previous sibling are, we can offer a record skimmer for that.

This would make it impossible for any system, including Dovahscript, to give an idle an invalid hierarchy position after we've built the full idle tree; and it would do so while still making it possible to know what parent and previous-sibling an idle *wanted* to have (i.e. it would still be possible to identify and diagnose an invalid hierarchy position). In other words, our systems wouldn't have the gaping hole that they presently have.

(This design has complications regarding use info. Say an idle's relationships are invalid, and its desired parent is defined in the active file and deleted after we load. Suppose a new form is created, recycling the form ID of the desired parent. If something queries its desired parent and previous sibling, how do we make sure we don't erroneously return the new form? Maybe a better design would be to store the desired parent and previous sibling on the idle, as is done currently, but don't allow them to be modified except: to sever them upon deletion of either form; or to update them, when the idle is re-saved.)
