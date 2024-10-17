# Scene UI

Bethesda's UI for editing scenes relies on the same flowchart library that they use for Dialogue Views, the friendly flowchart-based editor for branches, topics, and infos. The scene UI is less freeform; the flowchart library is used purely for the visuals, and scenes have a fairly rigid structure under the hood.

## Underlying data

The data is organized as follows:

* Scenes contain phases.
* Scenes contain actors.
* For each actor, a phase may contain at most one action of each type (dialogue, package, timer).

The data is displayed as follows:

* One column per phase: a grey box divided vertically.
  * The column has a header at the top consisting of at least one box (showing the phase number and, if it has one, the name). There may be a second or third box below it showing the phase conditions, if it has any.
  * If you scroll down so that the header is out of view, a smaller faded-mustard-colored box appears showing just the phase number. Its horizontal position is centered under the phase header.
* One row per actor: a white horizontal box overlaid over the grey background, and extending slightly past its left and right edges.
  * The actor row shows the alias name and its scene-relevant Actor Behavior settings along the left side. Scrolling horizontally keeps this text in view.
* Actor rows are split into three sub-rows: the top for dialogue actions, the middle for package actions, and the bottom for timer actions.

The data is serialized as follows:

* Scenes contain a flat list of phases.
* Scenes contain a flat list of actors.
* Scenes contain a flat list of actions. Actions specify the actor to which they apply, and the start and end phases' one-based indices.

### Action

Actions are numbered in the order they're created in.

#### Dialogue

Infos have `[S] ` prefixed if they have a fragment script.

## Interactions

### Phases

Double-clicking the phase header or its "sticky" number will open the phase options dialog box.

Single-clicking the phase header selects it. You can then use the horizontal resize handles to change the phase's display width. (The other resize handles, including the diagonals, are inoperable.)

The "sticky" number does nothing when single-clicked; it's non-solid.

The Del and Enter keys do nothing.

### Actors

The actor boxes are not directly interactable.

### Actions

Double-clicking the action header opens the action's dialog box.

Single-clicking the action selects it. You can then use the horizontal resize handles to try and change the action's start and end phases. (The other resize handles are operable, but do nothing once released.)

Dragging an action allows you to move it around. Upon releasing it, its start and end phases will be set to any column that it overlaps (which means that dragging it is not strictly a "move" operation; you can end up enlarging the action if you drag it, say, rightward enough to extend into another phase but not rightward enough to no longer overlap its start phase).

The Del and Enter keys do nothing.

## Layout rules

At 100% zoom, the graph is inset from the view by 30px on both axes. The phase grid is indented by 20px (and the action rows are not similarly indented, so they're outdented relative to the phase grid).

### Phases

The width of a Phase is ironclad; they don't widen or shrink to fit their contents. They have a very narrow minimum width &mdash: two grid cells' width, roughly 24px or 32px at 100% zoom; you can't resize them smaller than that. The width includes the entire grey column, not just the phase header's inner area. Form data stores the phase width.

A phase header is always two lines tall. The first line lists the phase number; the second, the name (if there is one). The phase name shown in the header does not word-wrap, and is clipped if too wide.

The condition boxes are always four lines tall, each, with "Start condition:" or "Completion condition:" on the first line and conditions beginning on the second. Conditions are word-wrapped and clipped. Condition boxes are not aligned with one another: if Phase 1 has both start and completion conditions, but Phase 2 has only completion conditions, then the Phase 1 start conditions will be at the same Y-position as the Phase 2 completion conditions.

### Actors

The height of an actor is dependent on the total heights of the actions therein. The minimum height is large enough to show the alias name and actor behavior.

The actor name is fixed-position, being inset by 40px from the view's left edge (at 100% zoom). Without any horizontal scrolling, that works out to a 10px inset from the left edge of the actor row.

### Actions

The left edge of an action aligns with the left edge of its start phase, and the right edge of an action aligns with the right edge of its end phase.

The height of an action depends on its type and contents:

* ***Dialogue:*** If "Show all text" is enabled for the view, then Info text will word-wrap, for example, expanding an action's height.
* ***Package:*** The editor IDs of packages is truncated with an ellipsis, and does not word-wrap. The layout preferentially truncates the editor ID first, and then the form ID. The form ID is clipped, without an ellipsis.
* ***Timer:*** The text of the action box word-wraps, but the action box remains at a fixed height, clipping its contents if they don't fit.