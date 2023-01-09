#pragma once
#include <vector>
#include <QObject>
#include "helpers/resizable_grid.h"
#include "helpers/singleton_ex.h"
#include "helpers/vector3.h"
#include "dovah/form_stub.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/ObjectReference.h"
#include "nif/file.h"
#include "vulkan/rendered_nif.h"
#include "vulkan/scene_entity_handle.h"

#include "./enums/bounds_generation_source.h"
#include "./enums/camera_speed_flags.h"
#include "./grid_definitions.h"

class DKVulkanView;
namespace dovah {
   namespace loaded_forms {
      class Landscape;
   }
}
namespace vulkanDK {
   class surface_renderer;
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
            camera_speed_flags camera_speed;
            struct {
               std::vector<selected_refr_info> refs;
            } selection;
         } state;

         vulkanDK::rendered_bounds_handle _make_bounds_for(const refr&, bounds_generation_source& out);
         refr* _get_loaded_refr_info(const dovah::form_stub&);
         cell* _get_loaded_cell_info(const dovah::form_stub&);
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

         std::vector<dovah::form_stub*> get_selected_refs() const;

         inline size_t cell_grid_size() const noexcept {
            return this->loaded_cells.length();
         }

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
   };
}