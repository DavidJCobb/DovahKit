
# `DKScopedProxyModel`

A proxy model that can be scoped to a given index, such that that index is displayed either as the proxy root, or as the only child of the proxy root. In other words, you can have one large model, and multiple proxies that show just individual parts of that model.

## Known issues

* If, within the source model, you drag-and-drop the proxy's current scope, then the proxy will lose track of the scope. This is impossible to fix.

  This problem is the result of Qt's drag-and-drop implementation being reprehensible spaghetti. All drag-and-drop operations &mdash; even those using `Qt::MoveAction`, and even strictly internal moves &mdash; are implemented by the default model implementations as copy operations which insert wholly new rows within the destination model. The source *view* then manually checks if the operation was a move and, if so, removes the originally dragged rows from the source model. The source model has no actual involvement in the drag-and-drop process, and thus can't offer any signals that would allow us to tell a drag-move apart from a removal followed by an insertion. The decision to implement internal moves as copies and deletions is obviously flawed, breaks `QPersistentModelIndex`es, and is too blisteringly stupid to work around.

  (Incidentally, this would also mean that a model that implements drag-move operations as *actual move operations*, rather than copy operations, would be at risk of the moved items being deleted instantly after the move.)

  This issue affects Qt 5, and Qt 6 as of version 6.10.1.