
# Split up the Worldedit core

`worldedit::core` is a god object in the same vein as the `surface_renderer` during the alpha launch; it's not nearly *as* egregious, but it's... pretty bad. I'd like to split it up; here are some thoughts:

* Component to manage loading and unloading, and to manage the renderer in general
* Component to manage the camera
* Component to manage the current ref-pick task
* Component to manage the current selection
* Component to manage running tools on behalf of Worldinput

Worldedit's current content could be split up as follows:

* `worldedit::components::loaded_area`
  * Nested classes
    * `...::loaded_cell` (previously `worldedit::cell`)
    * `...::loaded_ref`  (previously `worldedit::refr`)
  * Data members
    * `target_area`
    * `loaded_cells`
    * `loaded_refs`
  * Member functions
    * `_update_grid_size_from_inis`
    * `_make_bounds_for`
    * `_get_loaded_refr_info`
    * `_get_loaded_cell_info`
    * `_unload_refr`
    * `_unload_cell`
    * `_unload_all_cells`
    * `_load_refr`
    * `_load_cell`
    * `_on_renderer_nif_batch_loaded`
    * `_set_current_area_impl`
    * `_resize_cell_grid`
    * `set_current_area`
    * `is_cell_loaded`
    * `is_current_cell`
    * `is_ref_loaded`
    * `are_coordinates_outside_current_space`
    * `raycast_at`
    * `cell_grid_size`
* `worldedit::components::camera`
  * Member functions
    * `_center_camera_on_cell`
    * `adjust_camera`
    * `orbit_camera`
    * `translate_camera`
* `worldedit::components::ref_picking`
  * Member functions
    * `begin_pick_ref`
    * `cancel_pick_ref`
    * `attempt_pick_ref(passkey)`
* `worldedit::components::selection`
  * Member functions
    * `_get_primary_selected_refr_info`
    * `is_ref_selected`
    * `get_selection_centroid`
    * `get_selection_count`
    * `get_selected_refs`
    * `try_adjust_selection_coordinates`
    * `try_scale_selection`
    * `_finalize_refr_scaling`
    * `_finalize_selected_refr_scaling`
    * `_select_ref`
    * `_on_ref_deselected`
* `worldedit::components::tool_runner`
  * `get_editor_mode`
  * `get_edit_gizmo_frame`/`set_edit_gizmo_frame`
  * `get_edit_gizmo_mode`/`set_edit_gizmo_mode`
  * `_debug_dump_landscape_raycast`
* `worldedit::core`
  * Member functions
    * `center_on_refr`
    * `get_raycast_vectors`
