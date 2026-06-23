
# Useful editor tools and options

## Tools

* A keybind which replaces the current selection with the enable state parent of the current selection. (Fail if no selected ref has an enable state parent, or if, of all the selected ref that have enable state parents, those objects don't all have the same parent.) Use case is hiding Civil War cruft in cities; it can be hard to find the Civil War enable/disable markers when looking at cities from far above, and it'd be nice to be able to just select some Civil War rubble, press a key to change the selection to the enable marker, and then be able to hide all of that marker's enable children.

### Tool options

* Rotate selected refs

  * Control the pivot about which the selection rotates. (Centroid of all selected objects; pivot of some individual ref that is considered the "current" ref; etc.. Could add a keybind to cycle which ref, among the selections, is the "current" ref; IIRC I actually had something like that in Cobb Positioner.)
  
  * Along the same vein, the ability to define a keybind which rotates each selected ref individually about its own pivot, rather than rotating the group as a whole.


## Options

* An option to avoid opening ref dialogs directly overtop the Render Window if sufficient space is available on-screen next to the window (so you don't "lose" them under the Render Window if you click the Render Window).


## UI affordances

* An optional status bar on the Render Window; show X/Y/Z coords of the point the cursor is over.

