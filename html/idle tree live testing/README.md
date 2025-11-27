
JavaScript prototype for an idle treeview that:

* Accurately represents degenerate idle trees
* ...even in real-time as you create/move/delete idles

JavaScript used for rapid prototyping and iteration. Once I've nailed down the algorithms for live-editing the tree faithfully to degenerate trees, I'll port to C++.

Basic design:

* Define testcases as a list of idle forms
* Define idle forms as both serialized data and live treenode state/connections
* Implement some way to move idles (either drag-moving, or something with buttons e.g. "click button to move, then click button at destination")
* When an idle is moved/deleted/etc., we run the algorithm for making that change, including updating form data. *Then* we spawn a second treeview and run the "build all idle trees" algorithm again on the updated form data, and display the two treeviews side by side. If the two treeviews are identical, then it means we've properly made the edit and calculated the consequences given that testcase.

Reason for all this is because degenerate trees are hard to represent. For example, if idle A tries to be in two places at once, but is knocked out of one of those places by idle B, then moving or deleting B can cause A to spontaneously manifest in the place where B originally was, without ceasing to also be in the place it [A] originally managed to be.

## ESP files

ESP files are provided to mimic the testcases in `testcases.js`. These files must be loaded in order, but the Creation Kit relies on files' Date Modified to control load order, so you may have to fiddle with the timestamps.

Failing to load the files in the right order will cause spurious CK errors and a loaded idle tree that is not as designed. DovahKit enforces order based on file headers' listed dependencies rather than blindly trusting the timestamps, so this problem isn't something I need or plan to account for when implementing idle support.

These ESP files were created using xEdit, but `TESTIdleTreesVarious04.esp` was hex-edited afterward to add extra `IDLE/ANAM` subrecords per the intended design of those testcases. As such, xEdit will probably mishandle that specific file (stripping the intentionally duplicated subrecords) should any need arise to edit it later on.