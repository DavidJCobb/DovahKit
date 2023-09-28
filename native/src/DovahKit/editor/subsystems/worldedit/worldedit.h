#pragma once
#include <vector>
#include <QObject>
#include "helpers/passkey.h"
#include "helpers/resizable_grid.h"
#include "helpers/singleton_ex.h"
#include "helpers/vector3.h"
#include "dovah/form_stub.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/ObjectReference.h"
#include "nif/file.h"
#include "vulkan/enums/gizmo_mode.h"
#include "vulkan/rendered_nif.h"
#include "vulkan/scene_entity_handle.h"

#include "./enums/bool_operation.h"
#include "./enums/bounds_generation_source.h"
#include "./enums/camera_orbit_target.h"
#include "./enums/camera_speed_flags.h"
#include "./enums/editor_mode.h"
#include "./enums/reference_frame.h"
#include "./gizmo_colors/gizmo_color_scheme.h"
#include "./grid_definitions.h"
#include "./raycast_result.h"

class DKVulkanView;
namespace dovah {
   namespace loaded_forms {
      class Landscape;
   }
}
namespace vulkanDK {
   namespace data {
      struct camera_coordinate_change;
   }
   class surface_renderer;
}

namespace dovahkit::subsystems::worldedit::tools {
   namespace tandem {
      class adjust_camera;
   }
   class debug_dump_landscape_details;
   class move_selection;
}

namespace dovahkit::subsystems::worldedit {
   class core;

   //
   // Subsystem for accessing game assets.
   //
   class core : public QObject, public cobb::singleton_ex<core> {
      Q_OBJECT;
      protected:
         core();
      public:
         using singleton_ex::get;
         using singleton_ex::get_or_create;

         using gizmo_mode = vulkanDK::gizmo_mode;

         struct coordinate_adjustment {
            struct {
               cobb::vector3<float> euler;
               reference_frame      frame = reference_frame::world;
            } rotate;
            cobb::vector3<float> translate;
         };

      protected:
         using form_stub       = dovah::form_stub;
         using loaded_land_ptr = dovah::loaded_form_ptr<dovah::loaded_forms::Landscape>;
         using loaded_refr_ptr = dovah::loaded_form_ptr<dovah::loaded_forms::ObjectReference>;

         //using refr_nif_ptr = std::unique_ptr<nifDK::file>;
         using refr_nif_ptr = std::unique_ptr<vulkanDK::rendered_nif, vulkanDK::rendered_nif::deleter>;

         struct selected_refr_info {
            selected_refr_info();
            selected_refr_info(form_stub* f, vulkanDK::rendered_bounds_handle h) : stub(f), handle(h) {}
            ~selected_refr_info();

            selected_refr_info(const selected_refr_info& o) = delete;
            selected_refr_info& operator=(const selected_refr_info&) = delete;

            selected_refr_info(selected_refr_info&& o) noexcept { *this = std::move(o); }
            selected_refr_info& operator=(selected_refr_info&&) noexcept;

            form_stub* stub = nullptr;
            vulkanDK::rendered_bounds_handle handle;
            bool update_on_nif_load = false;

            loaded_refr_ptr loaded_ref_info() const;
         };

         struct refr {
            form_stub*      stub = nullptr;
            loaded_refr_ptr form;
            refr_nif_ptr    nif;
            struct {
               vulkanDK::rendered_light_handle light;
            } vulkan_handles;
         };

         struct cell {
            dovah::form_stub* stub = nullptr; // CELL form
            loaded_land_ptr   land = nullptr; // LAND form
            struct {
               vulkanDK::rendered_landscape_handle landscape;
            } vulkan_handles;
         };

         using loaded_cell_grid = cobb::resizable_square_grid<cell, loaded_cell_grid_coord>;

         DKVulkanView* target_view = nullptr;
         struct {
            form_stub* cell  = nullptr;
            form_stub* world = nullptr;

            struct {
               int32_t x = 0;
               int32_t y = 0;
            } world_grid_pos;
         } target_area;
         loaded_cell_grid  loaded_cells;
         std::vector<refr> loaded_refs;
         //
         struct {
            double last_frame_delta = 0;

            camera_speed_flags camera_speed;
            editor_mode mode = editor_mode::objects;

            struct {
               reference_frame frame = reference_frame::world;
               gizmo_mode      mode  = gizmo_mode::none;
            } gizmo;

            struct {
               std::vector<selected_refr_info> refs;
            } selection;

            // for "Orbit Camera" tool
            cobb::vector3<float> previous_selection_pivot;
         } state;

         void _update_grid_size_from_inis();

         vulkanDK::rendered_bounds_handle _make_bounds_for(const refr&, bounds_generation_source& out);

         const refr* _get_loaded_refr_info(const dovah::form_stub&) const;
         refr* _get_loaded_refr_info(const dovah::form_stub&);
         cell* _get_loaded_cell_info(const dovah::form_stub&);

         const selected_refr_info* _get_primary_selected_refr_info() const;
         selected_refr_info* _get_primary_selected_refr_info();

         void _unload_refr(refr&, bool handle_deselection = true); // does not remove the refr from the loaded refs list; caller must do that
         void _unload_refr(dovah::form_stub&);
         void _unload_cell(cell&);
         void _unload_cell(dovah::form_stub*);
         void _unload_all_cells();
         bool _load_refr(dovah::form_stub&, cobb::vector3<float>& out_pos, cobb::vector3<float>& out_rot, bool& out_is_coc);
         void _load_cell(dovah::form_stub*, loaded_cell_grid_coord gx, loaded_cell_grid_coord gy);

         void _center_camera_on_cell(dovah::form_stub&);

         static void _on_renderer_nif_batch_loaded();
         void _on_renderer_attached();
         void _on_renderer_loss_imminent(vulkanDK::surface_renderer&);
         void _on_renderer_lost();
         void _update_default_land_textures();
         void _update_gizmo_colors(const gizmo_color_scheme&);

         void _set_current_area_impl(dovah::form_stub* cell_or_world, int32_t grid_x = 0, int32_t grid_y = 0);

         void _resize_cell_grid(size_t length);
         
      public:
         void center_on_refr(dovah::form_stub&);
         void set_current_area(dovah::form_stub* cell_or_world);
         void set_current_area(dovah::form_stub* world, int32_t grid_x, int32_t grid_y);
         void set_target_view(DKVulkanView&);

         void view_input_poll_handler(DKVulkanView&);

         bool is_cell_loaded(const dovah::form_stub*) const;
         bool is_current_cell(const dovah::form_stub*) const;
         bool is_ref_loaded(const dovah::form_stub*) const;
         bool is_ref_selected(const dovah::form_stub*) const;

         cobb::vector3<float> get_selection_centroid() const;
         size_t get_selection_count() const;
         std::vector<dovah::form_stub*> get_selected_refs() const;

         raycast_result raycast_at(int view_x, int view_y) const;

         constexpr size_t cell_grid_size() const noexcept {
            return this->loaded_cells.length();
         }

         constexpr editor_mode get_editor_mode() const {
            return this->state.mode;
         }

         bool get_camera_speed_flag(camera_speed_flag) const;
         void modify_camera_speed_flag(camera_speed_flag, bool_operation);
         //
         float get_camera_move_speed() const;

         constexpr reference_frame get_edit_gizmo_frame() const { return this->state.gizmo.frame; }
         constexpr gizmo_mode      get_edit_gizmo_mode() const { return this->state.gizmo.mode; }
         void set_edit_gizmo_frame(reference_frame);
         void set_edit_gizmo_mode(gizmo_mode);

         glm::mat3 get_frame_rotation_matrix(reference_frame) const;

         void adjust_camera(vulkanDK::data::camera_coordinate_change&);
         void orbit_camera(camera_orbit_target pivot, cobb::vector3<float> euler_radians);
         bool try_adjust_selection_coordinates(const coordinate_adjustment&);

         #pragma region Passkeyed functions for tools
         void _debug_dump_landscape_raycast(cobb::passkey<core, class tools::debug_dump_landscape_details>, const dovah::form_stub& landscape, const glm::vec3& hit_position);
         #pragma endregion

      protected:
         void _select_ref(refr&);
         void _on_ref_deselected(dovah::form_stub&);

      signals:
         void cellLoaded(dovah::form_stub&);
         void cellUnloaded(dovah::form_stub&);
         void currentAreaChanged(dovah::form_stub* interior_cell_or_world);
         void currentCellChanged(dovah::form_stub* interior);
         void currentWorldChanged(dovah::form_stub* world);
         void crossedIntoExteriorCell(dovah::form_stub* cell); // use if you need to know what cell is at the center of the loaded grid
         void refSelected(dovah::form_stub&);
         void refDeselected(dovah::form_stub&);
         void refSelectionChanged(dovah::form_stub&, bool selected);

      public slots:
         void setRefSelectionState(dovah::form_stub&, bool state);
         void toggleRefSelectionState(dovah::form_stub&);
         void deselectAllRefs();
         void replaceRefSelection(dovah::form_stub&);
         void setCellGridSize(int);

         void unloadAll();
   };
}