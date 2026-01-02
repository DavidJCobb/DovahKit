
# Cell View

The cell view window has a few problems.

* The "cell ref list" would more accurately be considered a "cell child list." The CK lists navmeshes and landscapes in here as well. We don't, which is a huge gap in functionality.

  * Worth remembering that when sorting by editor ID, navmeshes and landscapes (in that order) are sorted above refs.

* Maybe we *shouldn't* bundle all three classes related to the ref list (the model, the proxy, and the list widget) in a single file?
