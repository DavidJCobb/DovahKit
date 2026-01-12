
# Drag/drop node ID map

This template helps with implementing drag-and-drop operations for performing internal moves within a `QAbstractItemModel` subclass.

## Usage

* *Do not* implement `removeRows` and friends for your model.
* Add a `mutable` instance of this template to your model.
* When a model item is being dragged (i.e. in your `mimeData` getter), pass the item to `track` to get a unique ID, and serialize that ID into the drag data buffer.
* Use `get_by_id` to pull model items back out of a drag data buffer.
* When a model item is about to be destroyed, pass it to `untrack`.

The `drag_drop_nodes_by_id.h` file contains helper functions for actually encoding and decoding these node IDs into drag data buffers, and extracting node pointers from said buffers.

## Background

When writing a `QAbstractItemModel` subclass that stores a tree of model items, you may wish to make it possible to move items within your model by dragging and dropping them. However, Qt's default drag-and-drop implementation isn't entirely well-suited for the purpose:

* The models...
  * `QAbstractItemModel::itemData(const QModelIndex&)` is intended to return a list of all data roles that have been set for a given item.
  * `QAbstractItemModel::mimeData(const QModelIndexList&)` will loop over every item in the list. For each item data role present on each item, it'll write the item's row, column, and the item data into a `QDataStream`. The MIME type used for the resulting data is `application/x-qabstractitemmodeldatalist`.
* The widgets...
  * `QAbstractItemView` executes a drop by removing the source items from the source view, and then inserting new items into the destination view.

This has a number of flaws:

* All drag-and-drop operations, including internal moves within a model, are processed as removals and insertions. Among other things, this breaks any `QPersistentModelIndex`es that referred to the dragged items.
* The default implementation can't transfer over any data that can't be serialized into a `QDataStream`.
* For a wholly default drag-and-drop implementation, drag-moving subtrees basically can't work. No hierarchical information is encoded; children and descendants of the dragged items aren't encoded.

So how do we work around this and implement a sane way to perform internal moves? Well, first, the model has to actively avoid implementing `removeRows` and friends, so that `QAbstractItemView` can't destroy the source rows when it carries out the "drop" half of an internal-move drag-and-drop. Second, the drag data has to consist of unique identifiers for the dragged nodes.

The "obvious" unique identifier to use would be a `QPersistentModelIndex`. However, `QAbstractItemModel` has no way to track the lifetime of a drag operation, so it would never know when it could destroy the QPMIs. The brute-force alternative would be to give *every* model item a unique identifier. The middle ground is to generate unique IDs for model items whenever they are first dragged, and that is what this template does. Since we can't know for sure when an item is no longer being dragged (in particular, if a drag operation is canceled, or if a drop on some other destination is rejected), we consider these IDs permanent: they live as long as the item lives, and they will effectively never be recycled (barring overflow of an unsigned 64-bit integer, which would require a *lot* of dragging).

## Implementation notes

* Instances of `ui::model_utils::drag_drop_node_id_map` need to be `mutable` because `QAbstractItemModel::mimeData` is a `const` getter, but we need to generate the IDs we want on-demand.
