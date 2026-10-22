
# Making one-way uses bidirectional

DovahKit needs to track Use Info between `form_stub`s, and it must track uses bidirectionally. However, bidirectional use info can't usually be created using multi-threading.

When we initially index form stubs, we use multi-threading to have each form stub build its outbound use info (i.e. to have user forms start one-way tracking used forms). When, we use a single thread to loop over user forms and update their used forms, to have the latter track the former.

When we re-parent persistent refs from worldspaces' persistent cells to non-persistent cells, by the refs' coordinates, we multi-thread both sides of this process:

* `set_parent_cell_worker` updates refs (user forms) to no longer track the persistent cell, and to instead track the cell their coordinates would place in.
* `make_parenthood_bidirectional_worker` updates cells (used forms) to track the refs that have been newly-placed inside.