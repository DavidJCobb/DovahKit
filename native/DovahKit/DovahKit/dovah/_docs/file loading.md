
# File loading

## Overview

You start by constructing a `file_queue` of all the files you wish to load. Then, you construct a `file_load_order` to feed the queue into. The resulting FLO will be an appropriately sorted file list with all needed dependencies added and with basic rules checked (throwing exceptions on failure).

Once you have an FLO, you can feed it into an `active_load_order` to actually carry out the load.

## Classes

### `file_queue`

A flat list of files that you wish to load.

### `file_load_order`

A list of files to be loaded. Each added file has its header skimmed, to get a list of its dependencies and to determine its type (heavy/light; master/plug-in); based on this information, files are sorted and dependencies are implicitly added.

### `active_load_order`

The full collection of loaded data.

