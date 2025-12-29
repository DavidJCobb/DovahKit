#include "worldedit.h"
#include <algorithm> // std::swap
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "dovah/data/spaces/interior_cell_max_sane_bounds.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/exceptions/form_creation_failed.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/form_stubs/helpers/get_cell_landscape.h"
#include "dovah/form_stubs/helpers/get_worldspace_cell_by_grid.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/Landscape.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/forms/Worldspace.h"
#include "dovah/forms/components/extra_data.h"
#include "dovah/forms/components/model.h"
#include "dovah/utils/world_coordinate_to_grid_coordinate.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/assets.h"
#include "editor/subsystems/game_inis.h"
#include "helpers/qt/strings.h"
#include "helpers/type_traits/value_type_of.h"
#include "nif/notice_code_t.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"
#include "vulkan/config/scene_limits.h"
#include "vulkan/helpers/glm_transform_from_beth.h"
#include "vulkan/raycast.h"
#include "vulkan/rendered_light.h"
#include "vulkan/surface_renderer.h"
#include "widgets/DKVulkanView.h"

#include "editor/subsystems/options/core.h"
#include "editor/ini/main.h"
namespace {
   namespace worldedit_ini_settings {
      using namespace dovahkit::ini::main::worldedit;
   }
}
#include "editor/subsystems/game_inis.h"
#include "./gizmo_colors/edit_gizmo_color_scheme_manager.h"
#include "helpers/color/rgb.h"

#include "helpers/vector.h"
namespace {
   static constexpr const bool selection_vector_is_unordered = false;
}

#include "editor/subsystems/worldinput/core.h"
#include "./tool_system/all_tools_by_execution_order.h"
#include "./tool_system/tool_response_tuple.h"
#include "editor/subsystems/worldinput/builtin_control_schemes/debug_wasd.h"
#include "editor/subsystems/worldinput/builtin_control_schemes/reach.h"

#include "vulkan/data/camera_coordinate_change.h"
#include "vulkan/helpers/nif/set_root_transform.h"
#include "vulkan/helpers/glm_transform_from_beth.h"

#include "helpers/math/rotation/unit_conversion.h"

namespace {
   static constexpr bool require_complete_implementation = false;
}

namespace {
   static constexpr bool debug_log_area_load_unload = true;
   static constexpr bool debug_log_mesh_load_unload = false;
}

namespace {
   static constexpr auto max_selected_refr_count = vulkanDK::config::max_rendered_bounds;

   static constexpr const size_t minimum_grids_to_load = 5;
   static_assert(minimum_grids_to_load >= 1);
}

namespace dovahkit::subsystems::worldedit {
   #pragma region selected_refr_info
   core::selected_refr_info::selected_refr_info() {}
   core::selected_refr_info::~selected_refr_info() {
      this->handle.destroy();
   }

   core::selected_refr_info& core::selected_refr_info::operator=(selected_refr_info&& o) noexcept {
      std::swap(this->stub,   o.stub);
      std::swap(this->handle, o.handle);
      std::swap(this->update_on_nif_load, o.update_on_nif_load);
      return *this;
   }

   core::loaded_refr_ptr core::selected_refr_info::loaded_ref_info() const {
      if (this->stub)
         return this->stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      return {};
   }
   #pragma endregion

   core::core() : QObject(nullptr) {
      #pragma region uGridsToLoad options
      {
         auto& subsys = dovahkit::subsystems::options::core::get();
         QObject::connect(&subsys, &dovahkit::subsystems::options::core::mainIniSettingChanged, this, [this](cobb::ini::setting& which) {
            if (&which == &worldedit_ini_settings::uLoadedGridSize || &which == &worldedit_ini_settings::bLoadedGridSizeOverrideFromSkyrimINI) {
               this->_update_grid_size_from_inis();
            }
         });
      }
      {
         auto& ini     = editor::game_inis::get_skyrim();
         auto* setting = ini.setting("General", "uGridsToLoad");
         if (setting) {
            QObject::connect(setting, &cobb::qt::ini::Setting::valueChanged, this, [this]() {
               this->_update_grid_size_from_inis();
            });
         }
      }
      #pragma endregion
      this->loaded_cells.resize(1);
      this->_update_grid_size_from_inis();

      auto& core = DovahKitCore::get();
      QObject::connect(&core, &DovahKitCore::dataAcquireComplete, this, &core::_update_default_land_textures);
      QObject::connect(&core, &DovahKitCore::dataAbandonImminent, this, &core::unloadAll);
      QObject::connect(&core, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* form, bool just_flagging) {
         if (form->form_type == dovah::form_type::cell) {
            this->_unload_cell(form);
            static_assert(!require_complete_implementation, "TODO: Interior cells: if the current cell is unloaded, reset lighting/fog params for the renderer.");
            return;
         }
         if (dovah::form_type_is_reference(form->form_type)) {
            this->_unload_refr(*form);
            return;
         }
         if (dovah::form_type_is_base_form(form->form_type)) {
            static_assert(!require_complete_implementation, "TODO: Find all loaded refs using this base form, and update them (show error NIF).");
            return;
         }
      });
      QObject::connect(&core, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
         if (form->form_type == dovah::form_type::cell) {
            if (!this->is_cell_loaded(form))
               return;
            static_assert(!require_complete_implementation, "TODO: Interior cells: check for changes to lighting params, and update Vulkan state.");
            static_assert(!require_complete_implementation, "TODO: Exterior cells: check for changes to region, water height, etc., and update as needed.");
            return;
         }
         if (dovah::form_type_is_reference(form->form_type)) {
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: Check for changes to render-relevant REFR fields, and update as needed.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: If we're in an exterior and an unselected REFR is moved out of the loaded area, unload the REFR.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: If we're in an exterior and a selected REFR is moved to another world or an interior, unload the REFR.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is NOT loaded: If we're in an exterior and the REFR is moved into the loaded area, load it.");
            return;
         }
         if (dovah::form_type_is_base_form(form->form_type)) {
            static_assert(!require_complete_implementation, "TODO: Check if render-relevant properties (i.e. model; light data) have changed. If so, find all loaded refs using this base form, and update them.");
            return;
         }
      });

      {
         auto& mgr = gizmo_color_scheme_manager::get_or_create();
         QObject::connect(&mgr, &gizmo_color_scheme_manager::colorSchemeChanged, this, [this](const gizmo_color_scheme& scheme) {
            this->_update_gizmo_colors(scheme);
         });
      }
   }

   void core::_update_grid_size_from_inis() {
      bool use_game_ini = worldedit_ini_settings::bLoadedGridSizeOverrideFromSkyrimINI.get_current_value<bool>();
      auto my_own_grids = worldedit_ini_settings::uLoadedGridSize.get_current_value<unsigned int>();

      if (!use_game_ini) {
         if (my_own_grids < minimum_grids_to_load)
            my_own_grids = minimum_grids_to_load;
         this->setCellGridSize(my_own_grids);
         return;
      }

      uint32_t grid = 5;

      auto& ini = editor::game_inis::get_skyrim();
      auto* setting = ini.setting("General", "uGridsToLoad");
      if (setting) {
         grid = setting->currentValue().toInt();
         if (grid < minimum_grids_to_load)
            grid = minimum_grids_to_load;
      }
      this->setCellGridSize(grid);
   }

   vulkanDK::rendered_bounds_handle core::_make_bounds_for(const refr& item, bounds_generation_source& src) {
      src = bounds_generation_source::undefined;
      //
      vulkanDK::rendered_bounds_handle handle;
      if (!this->target_view)
         return {};
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return {};
      assert(item.form);
      //
      glm::mat4 transform = vulkanDK::glm_transform_from_beth(item.form->position, item.form->rotation, item.form->get_scale());
      glm::vec3 bounds_min;
      glm::vec3 bounds_max;
      if (item.nif && item.nif->did_load_succeed()) {
         auto& bnd = item.nif->bounds;
         bounds_min = { bnd.min.x, bnd.min.y, bnd.min.z };
         bounds_max = { bnd.max.x, bnd.max.y, bnd.max.z };
         src = bounds_generation_source::nif;
      } else {
         //
         // TODO: Pull OBND and use it.
         //
         bounds_min = { -128, -128, -128 };
         bounds_max = {  128,  128,  128 };
         src = bounds_generation_source::none;
      }
      return sr->add_bounds(bounds_min, bounds_max, transform);
   }
   const core::refr* core::_get_loaded_refr_info(const dovah::form_stub& stub) const {
      for (auto& item : this->loaded_refs)
         if (item.stub == &stub)
            return &item;
      return nullptr;
   }
   core::refr* core::_get_loaded_refr_info(const dovah::form_stub& stub) {
      return const_cast<core::refr*>(std::as_const(*this)._get_loaded_refr_info(stub));
   }
   core::cell* core::_get_loaded_cell_info(const dovah::form_stub& stub) {
      for (auto& item : this->loaded_cells)
         if (item.stub == &stub)
            return &item;
      return nullptr;
   }


   const core::selected_refr_info* core::_get_primary_selected_refr_info() const {
      auto& list = this->state.selection.refs;
      if (list.empty())
         return nullptr;
      auto& item = list.back();
      assert(item.stub != nullptr);
      return &item;

   }
   core::selected_refr_info* core::_get_primary_selected_refr_info() {
      return const_cast<core::selected_refr_info*>(std::as_const(*this)._get_primary_selected_refr_info());
   }

   void core::_unload_refr(refr& refr, bool handle_deselection) {
      auto* stub = refr.stub;
      if (handle_deselection) {
         //
         // This function can optionally handle deselection as well. There are cases 
         // where the caller may want to do that itself -- for example, when unloading 
         // refs matching some criteria en masse, it may be cheaper to manually loop 
         // over the while selection list one time, instead of looping once per ref.
         //
         auto& list = this->state.selection.refs;
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [&stub](const cobb::value_type_of<decltype(list)>& item) { return item.stub == stub; }
            ),
            list.end()
         );
      }
      auto* sr = this->target_view->surfaceRenderer();
      if (this->target_view) {
         if (auto* sr = this->target_view->surfaceRenderer()) {
            //
            // Remove the ref and associated state from the Vulkan renderer.
            //
            if (auto& h = refr.vulkan_handles.light; !h.empty()) {
               h.destroy();
            }
            if (refr.nif)
               sr->remove_nif(*refr.nif);
         }
      }
      refr.nif.reset();
      refr.stub = nullptr;
      refr.form = nullptr;
   }
   void core::_unload_refr(dovah::form_stub& stub) {
      auto&  list = this->loaded_refs;
      size_t size = list.size();
      size_t i    = 0;
      for (; i < size; ++i)
         if (list[i].stub == &stub)
            break;
      if (i >= size)
         return;
      this->_unload_refr(list[i]);
      list.erase(list.begin() + i);
   }
   void core::_unload_cell(cell& loaded) {
      auto* cell = loaded.stub;
      //
      if (auto& handle = loaded.vulkan_handles.landscape; !handle.empty())
         handle.destroy();
      loaded.land = nullptr;
      loaded.stub = nullptr;
      //
      loaded = {};
      if (!cell)
         return;
      if constexpr (debug_log_area_load_unload) {
         qDebug("[Worldedit][_unload_cell] %08X", cell->formID);
      }
      //
      // Deselect any refs inside of this cell:
      //
      {
         auto& list = this->state.selection.refs;
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [&cell](const cobb::value_type_of<decltype(list)>& item) { return item.stub->get_parent_form() == cell; }
            ),
            list.end()
         );
      }
      //
      // Unload any refs inside of this cell:
      //
      auto& list = this->loaded_refs;
      for (auto& refr : list) {
         auto* stub = refr.stub;
         auto* parent = stub->get_parent_form();
         if (parent != cell)
            continue;
         this->_unload_refr(refr, false);
      }
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [cell](refr& item) {
               return item.stub == nullptr;
            }
         ),
         list.end()
      );
      //
      // Signals:
      //
      emit this->cellUnloaded(*cell);
   }
   void core::_unload_cell(dovah::form_stub* cell) {
      auto& list = this->loaded_cells;
      auto  it = std::find_if(list.begin(), list.end(), [cell](const core::cell& item) { return item.stub == cell; });
      if (it == list.end())
         return;
      this->_unload_cell(*it);
      return;
   }
   void core::_unload_all_cells() {
      //
      // Let's start by manually clearing all selections and unloading all REFRs. We 
      // could just have `_unload_cell` do it, but doing it here is more optimal: we 
      // already know that we want to unload all CELLs and REFRs, so why even bother 
      // searching the full ref list for each ref individually when what we want is 
      // to blow away the whole list?
      // 
      // In addition, handling REFRs here also deals with an important edge-case: a 
      // REFR can be selected and dragged into an unloaded CELL, in which case we don't 
      // force-load the cell. If we rely entirely on `_unload_cell` to unload refs, then 
      // we miss those refs.
      // 
      // This function is the key cleanup function for Worldedit: if data is unloaded 
      // (e.g. because the user is loading a new set of ESP files), we tell Worldedit 
      // to load a nullptr area, which calls this function (thereby releasing all of 
      // our pointers to form stubs and loaded game data) and then sets a few other 
      // things up (e.g. the debug grid shown when in no loaded area).
      //
      this->state.selection.refs.clear();
      {
         auto& list = this->loaded_refs;
         for (auto& item : list) {
            this->_unload_refr(item, false);
         }
         list.clear();
      }

      //
      // TODO: When we implement navmeshing, we'll want to release navmesh data here 
      // as well.
      //

      for (auto& item : this->loaded_cells)
         this->_unload_cell(item);
   }
   bool core::_load_refr(dovah::form_stub& stub, cobb::vector3<float>& out_pos, cobb::vector3<float>& out_rot, bool& out_is_coc) {
      #if _DEBUG
         if (stub.get_content_if_loaded()) {
            for (const auto& item : this->loaded_refs)
               assert(item.stub != &stub && "Don't call _load_refr on refs that are already loaded!");
         }
      #endif

      auto* base = dovah::form_stub_helpers::get_base_form(&stub);
      if (!base)
         return false;
      //
      auto* sr = this->target_view->surfaceRenderer();
      if (base->form_type == dovah::form_type::light) {
         auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
         if (!loaded)
            return false;
         static_assert(!require_complete_implementation, "TODO: Remember the light object (ideally as a handle of some kind), so we can clean it up on cell unload.");
         auto h = sr->add_light(*loaded);
         if (!h.empty()) {
            auto& item = this->loaded_refs.emplace_back();
            item.stub = &stub;
            item.form = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            item.vulkan_handles.light = h;
            //
            out_pos = item.form->position;
            out_rot = item.form->rotation;
            out_is_coc = false;
            //
            return true;
         }
         return false;
      }
      //
      auto loaded_base = base->load();
      if (!loaded_base)
         return false;
      auto* form_model = loaded_base->get_model();
      if (!form_model || form_model->model_path.empty())
         return false;
      //
      auto loaded = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      if (!loaded)
         return false;
      //
      float scale = loaded->get_scale();
      //
      auto* model = sr->add_nif(*base, *form_model, loaded->position.to_struct<glm::vec3>(), loaded->rotation.to_struct<glm::vec3>(), scale);
      if (!model) {
         return false;
      }
      model->owning_form = &stub;
      //
      auto& item = this->loaded_refs.emplace_back();
      item.stub = &stub;
      item.form = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      item.nif.reset(model);
      //
      out_pos    = loaded->position;
      out_rot    = loaded->rotation;
      out_is_coc = base->formID == dovah::hardcoded_form_ids::COCMarkerHeading;
      return true;
   }
   void core::_load_cell(dovah::form_stub* cell, loaded_cell_grid_coord gx, loaded_cell_grid_coord gy) {
      if (!this->target_view)
         return;
      assert(cell && cell->form_type == dovah::form_type::cell);
      assert(this->loaded_cells.contains_coordinate(gx, gy));
      auto& loaded = this->loaded_cells.at(gx, gy);
      assert(!loaded.stub && "Why is a cell already in this spot?");
      if constexpr (debug_log_area_load_unload) {
         qDebug("[Worldedit][_load_cell] %08X", cell->formID);
      }
      loaded.stub = cell;
      loaded.land = nullptr;
      if (cell->is_exterior_cell()) {
         if (auto* land = dovah::form_stub_helpers::get_cell_landscape(cell)) {
            loaded.land = land->load().ptr_cast<dovah::loaded_forms::Landscape>();
         }
      }
      
      size_t refr_count = 0;
      //
      glm::fvec3 cell_position = { 0, 0, 0 };
      float cell_land_max = 0;
      //
      auto* sr = this->target_view->surfaceRenderer();

      if (loaded.land) {
         cell_land_max = loaded.land->maximum_height();
         //
         int32_t gx;
         int32_t gy;
         if (cell->get_grid_coordinates(gx, gy)) {
            cell_position.x = (float)gx * dovah::loaded_forms::Cell::side_length;
            cell_position.y = (float)gy * dovah::loaded_forms::Cell::side_length;
            cell_position.z = 0.0F; // our LAND loader resolves the base height
         }
         //
         loaded.vulkan_handles.landscape = sr->add_landscape(cell_position, *loaded.land);
      }

      //
      // This will be a rare case, and will generally happen if the user moves a selected 
      // REFR outside of the loaded area.
      //
      bool any_refs_already_loaded = false;
      for (const auto& item : this->loaded_refs) {
         if (!item.stub)
            continue;
         if (item.stub->get_parent_form() == cell) {
            any_refs_already_loaded = true;
            break;
         }
      }

      dovah::form_stub_helpers::for_each_child_form(cell, [this, sr, &refr_count, any_refs_already_loaded](dovah::form_stub* stub) {
         if (!dovah::form_type_is_reference(stub->form_type))
            return false;

         if (any_refs_already_loaded) {
            for (const auto& item : this->loaded_refs)
               if (item.stub == stub)
                  //
                  // This ref is already loaded. Loading it a second time will break things.
                  //
                  return false;
         }
         
         cobb::vector3<float> pos;
         cobb::vector3<float> rot;
         bool is_coc;
         //
         if (!this->_load_refr(*stub, pos, rot, is_coc))
            return false;
         ++refr_count;
         
         return false;
      });
      //
      // Signals:
      //
      emit this->cellLoaded(*cell);
   }

   void core::_center_camera_on_cell(dovah::form_stub& stub) {
      if (!this->target_view) {
         return;
      }

      auto* loaded = this->_get_loaded_cell_info(stub);

      glm::vec3 centroid = { 0, 0, 0 };
      glm::vec3 coc_pos  = { 0, 0, 0 };
      glm::vec3 coc_rot  = { 0, 0, 0 };
      size_t refr_count = 0;
      bool   found_coc_marker = false;
      //
      glm::fvec3 cell_position = { 0, 0, 0 };
      float cell_land_max = 0;
      //
      auto* sr = this->target_view->surfaceRenderer();
      if (loaded && loaded->land) {
         cell_land_max = loaded->land->maximum_height();
         //
         int32_t gx;
         int32_t gy;
         if (stub.get_grid_coordinates(gx, gy)) {
            cell_position.x = (float)gx * dovah::loaded_forms::Cell::side_length;
            cell_position.y = (float)gy * dovah::loaded_forms::Cell::side_length;
            cell_position.z = 0.0F; // our LAND loader resolves the base height
         }
      }
      for (auto& item : this->loaded_refs) {
         if (!item.stub || item.stub->get_parent_form() != &stub)
            continue;
         auto* base = dovah::form_stub_helpers::get_base_form(item.stub);
         if (!base)
            continue;

         cobb::vector3<float> pos = item.form->position;
         cobb::vector3<float> rot = item.form->rotation;
         bool is_coc = base->formID == dovah::hardcoded_form_ids::COCMarkerHeading;

         if (is_coc) {
            found_coc_marker = true;
            coc_pos = { pos.x, pos.y, pos.z };
            coc_rot = { rot.x, rot.y, rot.z };
         } else {
            ++refr_count;
            centroid += glm::vec3{ pos.x, pos.y, pos.z };
         }
      }
      //
      // Got the info we wanted.
      //
      if (found_coc_marker) {
         coc_pos.z += 160; // the CK uses a vertical offset as well
         sr->scene.camera.set_coordinates(
            coc_pos,
            glm::fvec3{ coc_rot.x, 0.0, coc_rot.z }
         );
      } else {
         if (stub.is_exterior_cell()) {
            auto pos = cell_position;
            pos += glm::fvec3{ 2048, 2048, cell_land_max + 1024.0F };
            sr->set_camera_position(pos);
         } else {
            centroid /= refr_count;
            sr->set_camera_position(centroid);
         }
      }
   }

   /*static*/ void core::_on_renderer_nif_batch_loaded() {
      glm::vec3 bounds_min;
      glm::vec3 bounds_max;

      auto& self = core::get();
      for (auto& item : self.state.selection.refs) {
         if (item.update_on_nif_load) {
            auto* ref_info = self._get_loaded_refr_info(*item.stub);
            assert(ref_info);

            auto& bnd  = ref_info->nif->bounds;
            bounds_min = { bnd.min.x, bnd.min.y, bnd.min.z };
            bounds_max = { bnd.max.x, bnd.max.y, bnd.max.z };

            item.handle->set_size(bounds_min, bounds_max);
         }
      }
   }
   void core::_on_renderer_attached() {
      this->_update_default_land_textures();
      {
         if (!this->target_view)
            return;
         auto* sr = this->target_view->surfaceRenderer();
         if (!sr)
            return;
         auto& hooks = sr->get_hooks();
         hooks.nif_batches.on_background_use_complete = &_on_renderer_nif_batch_loaded;
      }
      this->_update_gizmo_colors(
         gizmo_color_scheme_manager::get_or_create().get_current_color_scheme()
      );
   }
   void core::_on_renderer_loss_imminent(vulkanDK::surface_renderer& sr) {
      auto& hooks = sr.get_hooks();
      if (auto& hook = hooks.nif_batches.on_background_use_complete; hook == &_on_renderer_nif_batch_loaded)
         hook = nullptr;
   }
   void core::_on_renderer_lost() {
      //
      // Forcibly discard all handles to scene objects.
      //
      for (auto& item : this->state.selection.refs) {
         item.handle = {};
      }
      for (auto& item : this->loaded_refs) {
         item.vulkan_handles = {};
         if (auto* nif = item.nif.get()) {
            for (auto* block : nif->all_blocks)
               block->sever_all_vulkan_mesh_connections();
         }
      }
      for (auto& item : this->loaded_cells) {
         item.vulkan_handles = {};
      }
   }
   void core::_update_default_land_textures() {
      if (!this->target_view)
         return;
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return;
      if (!DovahKitCore::get().has_data())
         //
         // We won't know where to load textures from if DovahKit doesn't have game data 
         // loaded. That's how it knows what game and BSAs to pull files from.
         //
         return;
      //
      // Default land textures:
      //
      auto& ini = editor::game_inis::get_skyrim();
      //
      QString diffuse_path;
      QString normals_path;
      if (auto* setting = ini.setting("Landscape", "sDefaultLandDiffuseTexture")) {
         auto value = setting->currentValue();
         if (value.userType() == QMetaType::QString) {
            diffuse_path = value.toString();
            if (!diffuse_path.isEmpty())
               diffuse_path = QLatin1String("textures/Landscape/") + diffuse_path;
         }
      }
      if (auto* setting = ini.setting("Landscape", "sDefaultLandNormalTexture")) {
         auto value = setting->currentValue();
         if (value.userType() == QMetaType::QString) {
            normals_path = value.toString();
            if (!normals_path.isEmpty())
               normals_path = QLatin1String("textures/Landscape/") + normals_path;
         }
      }
      //
      sr->set_default_land_textures(diffuse_path, normals_path);
   }
   void core::_update_gizmo_colors(const gizmo_color_scheme& scheme) {
      if (!this->target_view)
         return;
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return;

      auto axis_x    = ((cobb::color::rgb_floats)scheme.axis_x).sRGB_to_linear();
      auto axis_y    = ((cobb::color::rgb_floats)scheme.axis_y).sRGB_to_linear();
      auto axis_z    = ((cobb::color::rgb_floats)scheme.axis_z).sRGB_to_linear();
      auto highlight = ((cobb::color::rgb_floats)scheme.highlight).sRGB_to_linear();

      sr->scene.gizmo_state.color_x = { axis_x.r, axis_x.g, axis_x.b };
      sr->scene.gizmo_state.color_y = { axis_y.r, axis_y.g, axis_y.b };
      sr->scene.gizmo_state.color_z = { axis_z.r, axis_z.g, axis_z.b };
      sr->scene.gizmo_state.color_highlight = {
         highlight.r,
         highlight.g,
         highlight.b,
         1.0, // TODO: we actually blend the highlight color within the renderer. should we let the user customize that?
      };
      sr->force_gizmo_color_update();
   }
   
   void core::_set_current_area_impl(dovah::form_stub* cell_or_world, int32_t grid_x, int32_t grid_y) {
      dovah::form_stub* cell  = nullptr;
      dovah::form_stub* world = nullptr;
      if (cell_or_world) {
         if (cell_or_world->form_type == dovah::form_type::cell) {
            cell = cell_or_world;
            if (this->target_area.cell == cell)
               return;
            //
            world = cell->get_parent_form();
            if (world && world->form_type != dovah::form_type::worldspace)
               world = nullptr;
         } else if (cell_or_world->form_type == dovah::form_type::worldspace) {
            world = cell_or_world;
         } else {
            assert(false && "Worldedit was asked to load an area, but the provided form is not a cell or worldspace.");
         }
      }
      //
      if (world) {
         if constexpr (debug_log_area_load_unload) {
            qDebug("[Worldedit] Moving to worldspace [WRLD:%08X]...", world->formID);
         }
         //
         bool world_changed = false;
         if (this->target_area.cell || this->target_area.world != world) {
            if constexpr (debug_log_area_load_unload) {
               qDebug("[Worldedit] Different area; unloading all cells...");
            }
            world_changed = true;
         }
         this->target_area.cell  = nullptr;
         this->target_area.world = world;
         //
         auto& gp_now = this->target_area.world_grid_pos;
         auto  gp_old = gp_now;
         if (cell) {
            if (!cell->get_grid_coordinates(gp_now.x, gp_now.y)) {
               gp_now.x = 0;
               gp_now.y = 0;
            }
         } else {
            gp_now.x = grid_x;
            gp_now.y = grid_y;
            cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, grid_x, grid_y);
         }
         //
         auto diff_x = gp_now.x - gp_old.x;
         auto diff_y = gp_now.y - gp_old.y;
         if (!world_changed && diff_x == 0 && diff_y == 0) {
            return;
         }
         auto length = this->loaded_cells.length();
         if (!world_changed && std::abs(diff_x) < length && std::abs(diff_y) < length) {
            //
            // New area overlaps the old. Unload only the cells that have shifted out of the 
            // loaded grid.
            //
            if (diff_x || diff_y) {
               this->loaded_cells.shift_by(-diff_x, -diff_y, [this, world, &gp_now](core::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Unloading cell at (%d, %d) due to set-current-area...", (gp_now.x + x), (gp_now.y + y));
                  }
                  this->_unload_cell(data);
               });
            }
            this->loaded_cells.for_each([this, world, &gp_now](core::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
               if (data.stub)
                  return;
               auto* replace = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, gp_now.x + x, gp_now.y + y);
               if (replace) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Loading cell at (%d, %d) due to set-current-area...", (gp_now.x + x), (gp_now.y + y));
                  }
                  this->_load_cell(replace, x, y);
               }
            });
         } else {
            this->_unload_all_cells();
            //
            auto& lc = this->loaded_cells;
            for (loaded_cell_grid_coord y = lc.top(); y <= lc.bottom(); ++y) {
               for (loaded_cell_grid_coord x = lc.left(); x <= lc.right(); ++x) {
                  auto* replace = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, gp_now.x + x, gp_now.y + y);
                  if (replace) {
                     this->_load_cell(replace, x, y);
                  }
               }
            }
         }
         //
         // Move the camera.
         //
         if (cell) {
            this->_center_camera_on_cell(*cell);
         } else {
            //
            // The worldspace doesn't have a cell at these coordinates.
            //
            glm::vec3 camera = { 0, 0, 0 };
            camera.x = (float)gp_now.x * dovah::loaded_forms::Cell::side_length;
            camera.y = (float)gp_now.y * dovah::loaded_forms::Cell::side_length;
            camera.z = 0.0F;
            //
            // TODO: Get the worldspace's default land height, and position us above. Have us 
            // look down on the cell (or where a cell would be).
            //
            camera += glm::fvec3{ (dovah::loaded_forms::Cell::side_length / 2), (dovah::loaded_forms::Cell::side_length / 2), 1024.0F };
            if (this->target_view)
               this->target_view->surfaceRenderer()->set_camera_position(camera);
         }
         //
         if (world_changed)
            emit this->currentWorldChanged(world);
      } else {
         if constexpr (debug_log_area_load_unload) {
            if (cell) {
               qDebug("[Worldedit] Moving to [CELL:%08X]...", cell->formID);
            } else {
               qDebug("[Worldedit] Moving to nowhere...");
            }
         }
         //
         if (this->target_area.cell || this->target_area.world) {
            if constexpr (debug_log_area_load_unload) {
               qDebug("[Worldedit] Different area; unloading all cells...");
            }
            this->_unload_all_cells();
         }
         this->target_area.cell  = cell;
         this->target_area.world = nullptr;
         //
         if (cell) {
            assert(cell->form_type == dovah::form_type::cell && "Worldedit was asked to load a cell, but the provided form is not a cell.");
            this->_load_cell(cell, 0, 0);
            if constexpr (debug_log_area_load_unload) {
               qDebug("[Worldedit] Loading interior cell...");
            }
            this->_center_camera_on_cell(*cell);
            //
            // Signals:
            //
            emit this->currentCellChanged(cell);
         }
      }
      if (this->target_view) {
         auto* sr = this->target_view->surfaceRenderer();
         if (sr) {
            //
            // Hide the debug grid if we're loading any environment; show it if we're nowhere.
            //
            sr->set_debug_grid_visible(!cell && !world);
            if (!cell && !world) {
               //
               // If we're going nowhere, set the default camera coords.
               //
               sr->scene.camera.set_coordinates(
                  { 2.0F, 2.0F, 2.0F },
                  { glm::radians(-45.0F), 0, glm::radians(-45.0F) }
               );
            }
            //
            // Set lighting and fog params.
            //
            auto _to_vec = [](const dovah::loaded_forms::color_t& color) {
               return glm::vec3{ (float)color.r / 255.0F, (float)color.g / 255.0F, (float)color.b / 255.0F };
            };
            if (cell && !world) {
               auto  loaded = cell->load().ptr_cast<dovah::loaded_forms::Cell>();
               auto& sgs    = sr->scene.global_state;
               {
                  auto& lt = loaded->interior.lighting;
                  sgs.ambient_light_color = _to_vec(lt.ambient.base);
                  sgs.sun_color      = _to_vec(lt.directional.color);
                  static_assert(!require_complete_implementation, "TODO: sgs.sun_dir");
                  // TODO: sgs.sun_dir
                  sgs.fog_color_near = _to_vec(lt.fog.colors.near);
                  sgs.fog_color_far  = _to_vec(lt.fog.colors.far);
                  sgs.fog_plane_near = lt.fog.near;
                  sgs.fog_plane_far  = lt.fog.far;
                  sgs.fog_power      = lt.fog.power;
                  sgs.fog_max        = lt.fog.max;
                  sgs.interior_clip_distance = lt.fog.clip_distance;
               }
               if (loaded->interior.lighting_template) {
                  static_assert(!require_complete_implementation, "TODO: Load the LTMP and use its parameters.");
                  // TODO: load the LTMP and use its params
                  //       for now, we just reset some fields to safe defaults
                  sgs.interior_clip_distance = 0;
                  sgs.fog_plane_near = 0;
                  sgs.fog_plane_far  = 7000;
                  sgs.fog_power = 1;
                  sgs.fog_max   = 1;
               }
            } else if (world) {
               static_assert(!require_complete_implementation, "TODO: Pull lighting parameters from the appropriate Weather and time of day.");
               //
               auto& sgs = sr->scene.global_state;
               sgs.ambient_light_color = { 0.1, 0.1, 0.1 };
               sgs.sun_dir   = glm::normalize(glm::vec3{ 0.1, 0, -1 }); // vector from sun to world
               sgs.sun_color = { 1, 1, 1 };
               sgs.sun_space = glm::mat4(1);
               //
               sgs.fog_color_near = _to_vec({ 130, 150, 210 });
               sgs.fog_plane_near = 28000;
               sgs.fog_color_far  = _to_vec({ 130, 150, 210 });
               sgs.fog_plane_far  = 160000;
               sgs.fog_power      = 1.0F;
               sgs.fog_max        = 1.0F;
               sgs.interior_clip_distance = 0.0F;
            } else {
               //
               // We're loading "nowhere." Reset the default lighting.
               //
               auto& sgs = sr->scene.global_state;
               sgs.ambient_light_color = { 0.1, 0.1, 0.1 };
               //
               sgs.sun_dir = glm::normalize(glm::vec3{ 0.1, 0, -1 });;
               sgs.sun_color = { 1, 1, 1 };
               sgs.sun_space = glm::mat4(1);
               //
               sgs.fog_color_near = sgs.fog_color_far = { 0, 0, 0 };
               sgs.fog_plane_near = 0;
               sgs.fog_plane_far = 7000;
               sgs.fog_power = sgs.fog_max = 1.0F;
               sgs.interior_clip_distance = 0.0F;
            }
         }
      }
      //
      // Done!
      //
      emit this->currentAreaChanged(world ? world : cell);
   }

   void core::_resize_cell_grid(size_t length) {
      size_t prior_length = this->loaded_cells.length();
      if (length == prior_length)
         return;
      if (!this->target_area.world) {
         //
         // Fast path when viewing interiors:
         //
         if (!this->loaded_cells.empty()) {
            #if _DEBUG
               this->loaded_cells.for_each([](auto& item, auto x, auto y) {
                  if (x == 0 && y == 0)
                     return;
                  assert(!item.stub && "We weren't viewing a worldspace! There shouldn't be multiple cells loaded like this!");
               });
            #endif
            auto cell_info = std::move(this->loaded_cells.at(0, 0));
            this->loaded_cells = loaded_cell_grid(length);
            this->loaded_cells.at(0, 0) = std::move(cell_info);
         } else {
            this->loaded_cells = loaded_cell_grid(length);
         }
         return;
      }
      if (length < prior_length) {
         if constexpr (debug_log_area_load_unload) {
            qDebug("[Worldedit] Unloading %d cells due to grid size decreasing...", (prior_length * prior_length - length * length));
         }
      }
      this->loaded_cells.resize(length, [this](cell& item) {
         this->_unload_cell(item);
      });
      //
      // If the grid has been made larger, load the new cells.
      //
      if (length > prior_length) {
         const auto* world = this->target_area.world;
         if (world) {
            if constexpr (debug_log_area_load_unload) {
               qDebug("[Worldedit] Loading %d cells due to grid size increasing...", (length * length - prior_length * prior_length));
            }
            const auto& gp_now = this->target_area.world_grid_pos;
            this->loaded_cells.for_each([this, world, &gp_now](core::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
               if (data.stub)
                  return;
               auto* cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, gp_now.x + x, gp_now.y + y);
               if (cell) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Loading cell at (%d, %d) due to grid size increasing...", (gp_now.x + x), (gp_now.y + y));
                  }
                  this->_load_cell(cell, x, y);
               }
            });
         }
      }
   }

   void core::center_on_refr(dovah::form_stub& ref) {
      auto* cell = ref.get_parent_form();
      if (!cell || cell->form_type != dovah::form_type::cell)
         return;
      if (!this->is_cell_loaded(cell)) {
         this->set_current_area(cell);
      }
      //
      if (this->target_view) {
         if (auto* sr = this->target_view->surfaceRenderer()) {
            auto loaded = ref.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (loaded) {
               glm::fvec3 pos = { loaded->position.x, loaded->position.y, loaded->position.z };
               pos.z += 160;

               glm::fvec3 rot = { 0, 0, loaded->rotation.z };

               sr->scene.camera.set_coordinates(pos, rot);
            }
         }
      }
   }
   void core::set_current_area(dovah::form_stub* cell_or_world) {
      this->_set_current_area_impl(cell_or_world);
   }
   void core::set_current_area(dovah::form_stub* world, int32_t grid_x, int32_t grid_y) {
      if (world && world->form_type != dovah::form_type::worldspace) {
         #if _DEBUG
            __debugbreak(); // invalid argument
         #endif
         return;
      }
      this->_set_current_area_impl(world, grid_x, grid_y);
   }
   void core::set_target_view(DKVulkanView& view) {
      if (this->target_view == &view)
         return;
      assert(this->target_view == nullptr);
      this->target_view = &view;
      //
      worldinput::core::get().setTargetWidget(&view);
      //
      QObject::connect(&view, &DKVulkanView::rendererReady, this, &core::_on_renderer_attached, Qt::UniqueConnection);
      if (auto* sr = view.surfaceRenderer()) {
         //
         // Renderer is already ready.
         //
         this->_on_renderer_attached();
      }
      //
      QObject::connect(&view, &DKVulkanView::rendererErrorKillImminent, this, &core::_on_renderer_loss_imminent);
      QObject::connect(&view, &DKVulkanView::rendererKilledDueToError, this, &core::_on_renderer_lost);
      QObject::connect(&view, &QObject::destroyed, this, [this]() {
         worldinput::core::get().setTargetWidget(nullptr);
         this->target_view = nullptr;
         //
         this->_on_renderer_lost();
      });
   }

   void core::view_input_poll_handler(DKVulkanView& view) {
      bool scaled_refs_last_frame    = this->state.scaled_refs_this_input_poll;
      bool any_input_processing_done = false;
      this->state.scaled_refs_this_input_poll = false;

      if (&view != this->target_view)
         return;
      if (!view.isListeningForInput())
         return;
      //
      // Update edit gizmo mouseover state.
      //
      auto* sr = view.surfaceRenderer();
      if (sr) {
         auto cursor_pos = view.mapFromGlobal(QCursor::pos());

         vulkanDK::raycast rc(*sr);
         rc.test_flags = vulkanDK::raycast::test_flag::edit_gizmo;
         rc.set_screen_relative_raycast(cursor_pos.x(), cursor_pos.y());

         sr->do_raycast(rc);
         if (rc.result.hit) {
            sr->replace_gizmo_axis_highlighted(rc.result.gizmo.axis);
         } else {
            sr->clear_all_gizmo_axis_highlighting();
         }
      }
      {
         auto cursor_pos = view.mapFromGlobal(QCursor::pos()); // TODO: GET THIS FROM THE INPUT SYSTEM?

         //
         // Update input state.
         //
         tool_response_tuple results;
         double delta;
         any_input_processing_done = worldinput::core::get().doPerFrameInputProcessing(delta, results);
         this->state.last_frame_delta = delta;
         //
         if (!sr)
            //
            // Don't execute commands "blind." If there's no renderer, exit.
            //
            return;

         tools::all_tools_by_execution_order::for_each([&results]<typename Current>() {
            if constexpr (tools::is_tandem_invocation<Current>) {
               //
               // The "invoke in tandem" base class will dispatch all relevant response objects to the 
               // subclass.
               //
               Current::template invoke<Current>(results);
            } else if constexpr (tools::tool_response_or_tool_with_response<Current>) {
               if (results.has_member<Current>()) {
                  const auto& data = results.get_member<Current>();
                  Current::invoke(data);
               }
            }
         });
      }
      if (any_input_processing_done) {
         if (scaled_refs_last_frame && !this->state.scaled_refs_this_input_poll) {
            this->_finalize_selected_refr_scaling();
         }
      } else {
         this->state.scaled_refs_this_input_poll = scaled_refs_last_frame;
      }
      //
      // Done processing all tools.
      //
      if (sr) {  // Handle edit gizmo position and visibility
         glm::mat4  transform = glm::mat4(1);
         gizmo_mode mode_to_use = this->state.gizmo.mode;

         if (mode_to_use != gizmo_mode::none) {
            auto* sel_info = this->_get_primary_selected_refr_info();
            if (!sel_info) {
               mode_to_use = gizmo_mode::none;
            } else {
               auto loaded = sel_info->stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
               //
               auto rot = glm::fvec3{ 0, 0, 0 };
               switch (this->state.gizmo.frame) {
                  case reference_frame::current:
                  case reference_frame::world:
                     break;
                  case reference_frame::local:
                     rot = loaded->rotation.to_struct<glm::fvec3>();
                     break;
                  case reference_frame::camera:
                     if constexpr (require_complete_implementation) {
                        static_assert(!require_complete_implementation, "TODO: Implement camera-relative edit gizmo!");
                     }
                     break;
               }
               //
               transform = vulkanDK::glm_transform_from_beth(
                  loaded->position.to_struct<glm::fvec3>(),
                  rot,
                  1.0
               );
            }
         }
         sr->set_gizmo_mode(mode_to_use);
         sr->set_gizmo_transform(transform);
      }
      if (auto* world = this->target_area.world) {
         //
         // Process (un)loading cells as the camera moves.
         //
         const auto& camera_pos = sr->scene.camera.position();
         int32_t cgx = dovah::world_coordinate_to_grid_coordinate(camera_pos.x);
         int32_t cgy = dovah::world_coordinate_to_grid_coordinate(camera_pos.y);

         auto& gp = this->target_area.world_grid_pos;
         if (cgx != gp.x || cgy != gp.y) {
            auto diff_x = cgx - gp.x;
            auto diff_y = cgy - gp.y;
            auto length = this->loaded_cells.length();
            if (std::abs(diff_x) < length && std::abs(diff_y) < length) {
               //
               // New area overlaps the old. Unload only the cells that have shifted out of the 
               // loaded grid.
               //
               this->loaded_cells.shift_by(-diff_x, -diff_y, [this, world, cgx, cgy](core::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Unloading cell at (%d, %d) due to camera movement...", (cgx + x), (cgy + y));
                  }
                  this->_unload_cell(data.stub);
               });
               this->loaded_cells.for_each([this, world, cgx, cgy](core::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
                  if (data.stub)
                     return;
                  auto* replace = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, cgx + x, cgy + y);
                  if (replace) {
                     if constexpr (debug_log_area_load_unload) {
                        qDebug("[Worldedit] Loading cell at (%d, %d) due to camera movement...", (cgx + x), (cgy + y));
                     }
                     this->_load_cell(replace, x, y);
                  }
               });
            } else {
               this->_unload_all_cells();
               //
               auto& lc = this->loaded_cells;
               for (loaded_cell_grid_coord y = lc.top(); y <= lc.bottom(); ++y) {
                  for (loaded_cell_grid_coord x = lc.left(); x <= lc.right(); ++x) {
                     auto* replace = dovah::form_stub_helpers::get_worldspace_cell_by_grid(world, cgx + x, cgy + y);
                     if (replace) {
                        if constexpr (debug_log_area_load_unload) {
                           qDebug("[Worldedit] Loading cell at (%d, %d) due to camera movement...", (cgx + x), (cgy + y));
                        }
                        this->_load_cell(replace, x, y);
                     }
                  }
               }
            }
            gp.x = cgx;
            gp.y = cgy;
            //
            emit this->crossedIntoExteriorCell(this->loaded_cells.at(0, 0).stub);
         }
      }
   }

   bool core::is_cell_loaded(const dovah::form_stub* cell) const {
      if (!cell)
         return false;
      for (const auto& item : this->loaded_cells)
         if (item.stub == cell)
            return true;
      return false;
   }
   bool core::is_current_cell(const dovah::form_stub* cell) const {
      if (!cell)
         return false;
      if (this->target_area.world) {
         return cell == this->loaded_cells.at(0, 0).stub;
      }
      return cell == this->target_area.cell;
   }
   bool core::is_ref_loaded(const dovah::form_stub* ref) const {
      if (!ref)
         return false;
      for (const auto& item : this->loaded_refs)
         if (item.stub == ref)
            return true;
      return false;
   }
   bool core::is_ref_selected(const dovah::form_stub* ref) const {
      for (auto& item : this->state.selection.refs)
         if (item.stub == ref)
            return true;
      return false;
   }

   cobb::vector3<float> core::get_selection_centroid() const {
      cobb::vector3<float> out;
      size_t count = 0;

      if (this->get_editor_mode() == editor_mode::objects) {
         for (const auto& item : this->state.selection.refs) {
            assert(item.stub);
            auto loaded = item.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (loaded) {
               ++count;
               out += loaded->position;
            }
         }
      }

      if (count > 0)
         out /= count;

      return out;
   }
   size_t core::get_selection_count() const {
      switch (this->get_editor_mode()) {
         case editor_mode::objects:
            return this->state.selection.refs.size();
      }
      return 0;
   }
   std::vector<dovah::form_stub*> core::get_selected_refs() const {
      std::vector<dovah::form_stub*> out;
      out.reserve(this->state.selection.refs.size());

      for (const auto& item : this->state.selection.refs)
         if (item.stub)
            out.push_back(item.stub);

      return out;
   }

   bool core::are_coordinates_outside_current_space(float x, float y) const {
      if (this->target_area.world == nullptr) {
         auto* cell = this->target_area.cell;
         if (!cell || cell->is_exterior_cell()) {
            return false;
         }
         if (fabs(x) > dovah::spaces::interior_cell_max_sane_bounds || fabs(y) > dovah::spaces::interior_cell_max_sane_bounds) {
            return true;
         }
      } else {
         auto loaded = this->target_area.world->load().ptr_cast<dovah::loaded_forms::Worldspace>();
         if (loaded) {
            using world_flag = dovah::loaded_forms::Worldspace::world_flag;
            if (loaded->world_flags & (world_flag::small_world | world_flag::fixed_dimensions)) { // which of these flags are significant? both of them?
               //
               // "Small World" worldspaces have finite bounds, and don't auto-generate new cells when the 
               // player leaves those bounds. In fact, if the player leaves the bounds, then their character 
               // model gets accidentally removed from the scene graph, softlocking the game. I assume that 
               // objects placed out of bounds in these worldspaces would be similarly broken.
               //
               float grid_x = dovah::world_coordinate_to_grid_coordinate(x);
               float grid_y = dovah::world_coordinate_to_grid_coordinate(y);
               if (grid_x < loaded->bounds.min.x || grid_x >= loaded->bounds.max.x)
                  return true;
               if (grid_y < loaded->bounds.min.y || grid_y >= loaded->bounds.max.y)
                  return true;
            }
         }
      }
      return false;
   }

   raycast_result core::raycast_at(int view_x, int view_y) const {
      raycast_result out;
      
      if (!this->target_view)
         return out;
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return out;

      vulkanDK::raycast raycast(*sr);
      raycast.test_flags = vulkanDK::raycast::test_flag::all;
      raycast.set_screen_relative_raycast(view_x, view_y);
      sr->do_raycast(raycast);

      if (raycast.result.hit) {
         out.hit_position = raycast.result.hit.position;
      }
      out.view_position = QPointF((qreal)view_x, (qreal)view_y);
      switch (raycast.result.target) {
         using enum vulkanDK::raycast_hit_target;
         case none:
            break;
         case edit_gizmo:
            {
               auto& dst = out.target_info.edit_gizmo.axis;
               switch (raycast.result.gizmo.axis) {
                  using enum vulkanDK::axis3D;
                  using to = axis3D;
                  case x: dst = to::x; break;
                  case y: dst = to::y; break;
                  case z: dst = to::z; break;
               }
            }
            out.target_info.edit_gizmo.mode = raycast.result.gizmo.mode;
            break;
         case entity:
            {
               auto& ev = raycast.result.entity;
               if (std::holds_alternative<vulkanDK::rendered_landscape_handle>(ev)) {
                  auto handle = std::get<vulkanDK::rendered_landscape_handle>(ev);
                  if (!handle.empty()) {
                     for (const auto& info : this->loaded_cells) {
                        if (info.vulkan_handles.landscape == handle) {
                           out.target_info.form = &(info.land->stub);
                           break;
                        }
                     }
                  }
               } else if (std::holds_alternative<vulkanDK::rendered_mesh_handle>(ev)) {
                  auto handle = std::get<vulkanDK::rendered_mesh_handle>(ev);
                  if (!handle.empty()) {
                     if (auto* nif = handle->owning_nif; nif) {
                        auto* form = out.target_info.form = nif->owning_form;
                        if (form)
                           out.target_info.is_selected = this->is_ref_selected(form);
                     }
                  }
               }
            }
            break;
      }

      return out;
   }

   bool core::get_camera_speed_flag(camera_speed_flag flag) const {
      return this->state.camera_speed.test(flag);
   }
   void core::modify_camera_speed_flag(camera_speed_flag flag, bool_operation op) {
      auto& mask = this->state.camera_speed;
      switch (op) {
         using enum worldedit::bool_operation;
         case set_true:
            mask.set(flag);
            break;
         case set_false:
            mask.reset(flag);
            break;
         case invert:
            mask.flip(flag);
            break;
      }
   }
   //
   float core::get_camera_move_speed() const {
      float speed = worldedit_ini_settings::fCameraSpeedNormal.get_current_value<double>();
      
      if (this->get_camera_speed_flag(camera_speed_flag::boost))
         speed *= worldedit_ini_settings::fCameraSpeedMultBoost.get_current_value<double>();
      if (this->get_camera_speed_flag(camera_speed_flag::precision))
         speed *= worldedit_ini_settings::fCameraSpeedMultPrecision.get_current_value<double>();

      return speed;
   }

   void core::set_edit_gizmo_frame(reference_frame f) {
      if (f == reference_frame::current)
         return;
      this->state.gizmo.frame = f;
   }
   void core::set_edit_gizmo_mode(gizmo_mode m) {
      this->state.gizmo.mode = m;
   }

   glm::mat3 core::get_frame_rotation_matrix(reference_frame frame) const {
      if (frame == reference_frame::current) {
         frame = this->get_edit_gizmo_frame();
      }

      switch (frame) {
         case reference_frame::local:
            if (this->get_selection_count() != 0) {
               auto* info = this->_get_primary_selected_refr_info();
               if (info) {
                  auto loaded = info->loaded_ref_info();
                  if (loaded) {
                     return glm::mat3(vulkanDK::glm_transform_from_beth({ 0, 0, 0 }, loaded->rotation, 1.0));
                  }
               }
            }
            break;
         case reference_frame::camera:
            if (auto* sr = this->target_view->surfaceRenderer()) {
               auto& cs = sr->scene.camera;
               return cs.camera_rotation_matrix();
            }
            break;
      }
      return glm::mat3(1);
   }
   bool core::get_raycast_vectors(int screen_x, int screen_y, glm::vec3& out_ray_origin, glm::vec3& out_ray_direction) const {
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr) {
         out_ray_origin    = { 0, 0, 0 };
         out_ray_direction = { 0, 0, 0 };
         return false;
      }
      sr->surface_position_to_world_ray(screen_x, screen_y, out_ray_origin, out_ray_direction);
      return true;
   }

   void core::adjust_camera(vulkanDK::data::camera_coordinate_change& update) {
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return;
      sr->scene.adjust_camera(update);
   }
   void core::orbit_camera(camera_orbit_target target, cobb::vector3<float> euler_radians) {
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return;

      cobb::vector3<float> pivot;
      switch (target) {
         case camera_orbit_target::primary_selection:
            pivot = this->state.previous_selection_pivot; // TODO: What if the user has never selected anything before?
            break;
         case camera_orbit_target::selection_centroid:
            pivot = this->get_selection_centroid();
            break;
      }

      sr->scene.camera.arcball(pivot.to_struct<glm::vec3>(), euler_radians.to_struct<glm::vec3>());
   }
   void core::translate_camera(cobb::vector3<float> move, reference_frame frame) {
      auto* sr = this->target_view->surfaceRenderer();
      if (!sr)
         return;
      auto& cam = sr->scene.camera;

      if (frame == reference_frame::current) {
         frame = this->get_edit_gizmo_frame();
      }
      switch (frame) {
         case reference_frame::camera:
            cam.translate_relative(move.to_struct<glm::vec3>());
            break;
         case reference_frame::world:
            cam.translate_absolute(move.to_struct<glm::vec3>());
            break;
         case reference_frame::local:
            {
               auto mat = this->get_frame_rotation_matrix(frame);
               cam.translate_absolute(mat * move.to_struct<glm::vec3>());
            }
            break;
      }
   }
   bool core::try_adjust_selection_coordinates(const coordinate_adjustment& adjust) {
      if (this->target_area.cell == nullptr && this->target_area.world == nullptr) // no cell loaded
         return false;

      bool only_rotating_one_ref = this->get_selection_count() == 1;

      glm::mat4 centroid_inverse;
      glm::mat4 centroid_post_adjust;
      //
      cobb::vector3<float> single_ref_rotation;
      //
      {
         single_ref_rotation = adjust.rotate.euler;
         //
         auto f = adjust.rotate.frame;
         if (f == reference_frame::current)
            f = this->get_edit_gizmo_frame();
         //
         switch (f) {
            case reference_frame::camera:
            case reference_frame::local:
               {
                  auto mat = this->get_frame_rotation_matrix(f);
                  single_ref_rotation = single_ref_rotation.to_struct<glm::vec3>() * mat;
               }
               break;
         }

         if (!only_rotating_one_ref) {
            cobb::vector3<float> selection_centroid = this->get_selection_centroid();
            glm::mat4 centroid_transform;
            
            cobb::vector3<float> primary_selection_euler = {};
            if (auto* info = this->_get_primary_selected_refr_info()) {
               auto loaded = info->loaded_ref_info();
               if (loaded)
                  primary_selection_euler = loaded->rotation;
            }

            centroid_transform   = vulkanDK::glm_transform_from_beth(selection_centroid, primary_selection_euler, 1.0F); // scale is irrelevant here
            centroid_inverse     = glm::inverse(centroid_transform);
            centroid_post_adjust = vulkanDK::glm_transform_from_beth(selection_centroid, primary_selection_euler + single_ref_rotation, 1.0F); // scale is irrelevant here
         }
      }

      bool is_interior = false;
      if (this->target_area.world == nullptr) {
         assert(this->target_area.cell->is_exterior_cell() == false);
         is_interior = true;
      }

      if (this->state.mode == editor_mode::objects) {
         //
         // Check constraints.
         //
         if (is_interior) {
            for (const auto& sel_info : this->state.selection.refs) {
               assert(sel_info.stub);
               auto loaded = sel_info.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
               if (!loaded)
                  continue;
               
               cobb::vector3<float> pos_after;
               if (only_rotating_one_ref) {
                  //
                  // For a single ref, just keep things simple.
                  //
                  pos_after = loaded->position + adjust.translate;
               } else {
                  auto sel_world = vulkanDK::glm_transform_from_beth(loaded->position, loaded->rotation, 1.0F); // scale is irrelevant here
                  auto sel_pivot = centroid_inverse     * sel_world;
                  auto sel_final = centroid_post_adjust * sel_pivot;

                  pos_after = cobb::vector3<float>(sel_final[3]) + adjust.translate;
               }

               if (this->are_coordinates_outside_current_space(pos_after.x, pos_after.y))
                  return false;
            }
         }
         //
         // Apply movement if able.
         //
         for (auto& sel_info : this->state.selection.refs) {
            assert(sel_info.stub);
            auto loaded = sel_info.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (!loaded)
               continue;

            auto gx_prior = dovah::world_coordinate_to_grid_coordinate(loaded->position.x);
            auto gy_prior = dovah::world_coordinate_to_grid_coordinate(loaded->position.y);

            cobb::vector3<float> pos_after;
            cobb::vector3<float> rot_after;
            if (only_rotating_one_ref) {
               //
               // For a single ref, just keep things simple.
               //
               pos_after = loaded->position + adjust.translate;
               rot_after = loaded->rotation + single_ref_rotation; // TODO: this misses the code above that accounts for the reference frame!
            } else {
               auto sel_world = vulkanDK::glm_transform_from_beth(loaded->position, loaded->rotation, 1.0F); // scale is irrelevant here
               auto sel_pivot = centroid_inverse     * sel_world;
               auto sel_final = centroid_post_adjust * sel_pivot;

               pos_after = cobb::vector3<float>(sel_final[3]) + adjust.translate;
               glm::extractEulerAngleXYZ(sel_final, rot_after.x, rot_after.y, rot_after.z);
            }

            if (!is_interior) {
               auto gx_after = dovah::world_coordinate_to_grid_coordinate(pos_after.x);
               auto gy_after = dovah::world_coordinate_to_grid_coordinate(pos_after.y);
               if (gx_prior != gx_after || gy_prior != gy_after) {
                  //
                  // Re-parent the ref to the cell we're moving it into.
                  //
                  auto* destination_cell = dovah::form_stub_helpers::get_worldspace_cell_by_grid(this->target_area.world, gx_after, gy_after);
                  if (!destination_cell) {
                     //
                     // Create the destination cell.
                     //
                     try {
                        auto request = DovahKitCore::get().request_form_creation(dovah::form_type::cell);
                        request.set_parent_form(this->target_area.world);
                        request.cell_grid_coordinates = { .x = gx_after, .y = gy_after };
                        destination_cell = request.commit();
                     } catch (const dovah::exceptions::form_creation_failed& ex) {
                        #if _DEBUG
                           __debugbreak();
                        #endif
                        continue;
                     }
                     if (!destination_cell) {
                        #if _DEBUG
                           __debugbreak();
                        #endif
                        continue;
                     }
                     //
                     // We want to load the newly-created cell, if possible.
                     //
                     if (this->loaded_cells.contains_coordinate(gx_after, gy_after)) {
                        this->_load_cell(destination_cell, gx_after, gy_after);
                     }
                  }
                  sel_info.stub->set_parent_form(destination_cell);
               }
            }

            loaded->position = pos_after;
            loaded->rotation = rot_after;
            //
            auto transform_after = vulkanDK::glm_transform_from_beth(pos_after, rot_after, loaded->get_scale());
            //
            if (!sel_info.handle.empty()) {
               sel_info.handle->set_transform(transform_after); // update selection's drawn bounding box
            }
            if (auto* ref_info = this->_get_loaded_refr_info(*sel_info.stub)) {
               if (ref_info->nif && ref_info->nif->did_load_succeed()) {
                  //
                  // Update the REFR's rendered NIF and its constituent meshes.
                  //
                  vulkanDK::helpers::nif::set_root_transform(*ref_info->nif, transform_after);
               }
            }
         }
      }

      return true;
   }

   bool core::try_scale_selection(float mod, bool scale_all_together) {
      if (this->target_area.cell == nullptr && this->target_area.world == nullptr) // no cell loaded
         return false;

      constexpr const float scale_minimum   =   0.01F;
      constexpr const float scale_maximum   = 100.00F;

      constexpr const float epsilon = 0.00001F;

      float capped_mod = mod;
      float centroid_mod;
      if (fabs(capped_mod) < epsilon)
         return false;

      bool only_transforming_one_ref = this->get_selection_count() == 1;

      cobb::vector3<float> centroid_position;
      if (scale_all_together && !only_transforming_one_ref) {
         centroid_position = this->get_selection_centroid();
      }

      bool is_interior = false;
      if (this->target_area.world == nullptr) {
         assert(this->target_area.cell->is_exterior_cell() == false);
         is_interior = true;
      }

      //
      // Check all refs and limit the amount by which we scale, such that no ref is pushed 
      // above the max scale or below the min scale.
      //
      for (auto& sel_info : this->state.selection.refs) {
         assert(sel_info.stub);
         auto loaded = sel_info.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
         if (!loaded)
            continue;

         float current = loaded->get_raw_scale();
         {
            bool altered = false;
            if (current + capped_mod > scale_maximum) {
               capped_mod = scale_maximum - current;
               altered    = true;
            } else if (current + capped_mod < scale_minimum) {
               capped_mod = current - scale_minimum;
               altered    = true;
            }
            if (altered) {
               if (fabs(capped_mod) < epsilon)
                  //
                  // One or more refs has already hit the scale limit in whatever direction 
                  // we're altering the scale (increase/decrease). Fail here.
                  //
                  return false;
            }
         }
      }

      glm::mat4 centroid_post_adjust;

      //
      // If we're scaling multiple refs as a unit, such that their distances to one another are 
      // also scaled, then ensure we aren't pushing any of them into an area they're not allowed 
      // to be in. (This has to be done in a separate loop from above, because we have to know 
      // the final i.e. capped scale mod value.)
      //
      if (scale_all_together && !only_transforming_one_ref && is_interior) {
         centroid_mod = 1.0; // fallback for safety
         if (auto* primary = this->_get_primary_selected_refr_info()) {
            auto loaded = primary->stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (loaded) {
               float basis   = loaded->get_raw_scale();
               float desired = basis + capped_mod;
               centroid_mod = (desired / basis);
            }
         }

         for (auto& sel_info : this->state.selection.refs) {
            assert(sel_info.stub);
            auto loaded = sel_info.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
            if (!loaded)
               continue;

            auto diff = loaded->position - centroid_position; // vector from centroid to ref pos
            diff *= centroid_mod;

            auto pos_after = centroid_position + diff;
            if (this->are_coordinates_outside_current_space(pos_after.x, pos_after.y))
               return false;
         }
      }

      //
      // Apply the new scale factor.
      //
      for (auto& sel_info : this->state.selection.refs) {
         assert(sel_info.stub);
         auto loaded = sel_info.stub->load().ptr_cast<dovah::loaded_forms::ObjectReference>();
         if (!loaded)
            continue;
         loaded->set_scale(loaded->get_raw_scale() + capped_mod, false);

         if (scale_all_together && !only_transforming_one_ref) {
            auto diff = loaded->position - centroid_position; // vector from centroid to ref pos
            diff *= centroid_mod;

            auto pos_after = centroid_position + diff;
            loaded->position = pos_after;
         }

         auto transform_after = vulkanDK::glm_transform_from_beth(loaded->position, loaded->rotation, loaded->get_scale());

         if (!sel_info.handle.empty()) {
            sel_info.handle->set_transform(transform_after); // update selection's drawn bounding box
         }
         if (auto* ref_info = this->_get_loaded_refr_info(*sel_info.stub)) {
            if (ref_info->nif && ref_info->nif->did_load_succeed()) {
               //
               // Update the REFR's rendered NIF and its constituent meshes.
               //
               vulkanDK::helpers::nif::set_root_transform(*ref_info->nif, transform_after);
            }
         }
      }
      this->state.scaled_refs_this_input_poll = true;
      return true;
   }
   void core::_finalize_refr_scaling(dovah::form_stub& refr) {
      //
      // Refs can be uniformly scaled, but during gameplay, the scaling factor has a precision 
      // limit of 0.01 and a range of [0.01, 100.00]. (In actuality, it's stored as fixed-point 
      // at run-time.) We *want* to honor those limits... but doing so would break inputs for 
      // scaling refs. For example, you wouldn't be able to set a bind like "Hold this button 
      // to scale refs at a rate of 10%/s," because the amount you'd scale by *per frame* would 
      // be less than 0.01 (i.e. 1%).
      // 
      // The solution? Scaling is stored in a REFR's extra data as a single-precision float, so 
      // we can just store full-precision values in there. Once a scaling operation is done, we 
      // go back and update the ref to use the limited precision. We define "done" as the frame 
      // after the last frame any scaling operation occurred, or when a ref is deselected. We 
      // do it this way, using a state bool, so that outside code which wants to scale refs over 
      // time doesn't have to explicitly tell us when it's done.
      // 
      // Note that during rendering and such, we don't need to manually apply precision limits 
      // if we use the return value of ObjectReference::get_scale, which returns a limited value 
      // for us.
      //
      auto loaded = refr.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      if (!loaded)
         return;
      loaded->set_scale(loaded->get_scale(), true);
   }
   void core::_finalize_selected_refr_scaling() {
      for (auto& sel_info : this->state.selection.refs) {
         assert(sel_info.stub);
         this->_finalize_refr_scaling(*sel_info.stub);
      }
   }

   #pragma region Passkeyed functions for tools
   void core::_debug_dump_landscape_raycast(cobb::passkey<core, tools::debug_dump_landscape_details>, const dovah::form_stub& landscape, const glm::vec3& hit_position) {
      if (landscape.form_type != dovah::form_type::land) {
         qDebug("The hit form is not a landscape.");
         return;
      }

      vulkanDK::rendered_landscape_handle handle;
      for (auto& item : this->loaded_cells) {
         if (item.land && &(item.land->stub) == &landscape) {
            handle = item.vulkan_handles.landscape;
            break;
         }
      }
      if (handle.empty()) {
         qDebug("Landscape form %08X has no scene entity handle.", landscape.formID);
         return;
      }

      auto pos = hit_position - handle->frame_drawing_data.position;
      qDebug(
         "Hit landscape at (%g, %g, %g).",
         handle->frame_drawing_data.position.x,
         handle->frame_drawing_data.position.y,
         handle->frame_drawing_data.position.z
      );
      qDebug(" - Landscape-relative position: (%g, %g, %g)", pos.x, pos.y, pos.z);

      pos /= vulkanDK::rendered_landscape::vertex_distance;

      int x = pos.x;
      int y = pos.y;
      qDebug(" - Landscape-relative vertex row/col: (%d, %d)", x, y);

      if (x > 0 && y > 0 && x < 33 && y < 33) {
         //
         // TODO: console-print the nearest vertex's attributes.
         //
      }
   }
   #pragma endregion

   void core::_select_ref(refr& info) {
      auto& stub = *info.stub;

      auto& list = this->state.selection.refs;
      if (list.size() >= max_selected_refr_count) {
         return;
      }
      bounds_generation_source source_info;
      list.push_back({ &stub, _make_bounds_for(info, source_info) });
      if (source_info != bounds_generation_source::undefined && source_info != bounds_generation_source::nif) {
         list.back().update_on_nif_load = true;
      }

      if (info.form)
         this->state.previous_selection_pivot = info.form->position;

      emit this->refSelected(stub);
      emit this->refSelectionChanged(stub, true);
   }
   void core::_on_ref_deselected(dovah::form_stub& stub) {
      this->_finalize_refr_scaling(stub);

      if (auto* parent_cell = stub.get_parent_form()) {
         assert(parent_cell->form_type == dovah::form_type::cell);
         if (!this->is_cell_loaded(parent_cell)) {
            //
            // The user can have a selected ref that exists in an unloaded cell, if they've 
            // moved the ref beyond the loaded area while viewing a worldspace. In that case, 
            // we should unload any such refs once they're deselected, if they still aren't 
            // in a loaded cell at that time.
            //
            this->_unload_refr(stub);
         }
      }

      if (!this->state.selection.refs.empty()) {
         const auto& info = this->state.selection.refs.back();
         auto loaded = info.loaded_ref_info();
         if (loaded)
            this->state.previous_selection_pivot = loaded->position;
      }

      emit this->refDeselected(stub);
      emit this->refSelectionChanged(stub, false);
   }

   void core::setRefSelectionState(dovah::form_stub& stub, bool state) {
      auto* ref_info = this->_get_loaded_refr_info(stub);
      if (!ref_info)
         //
         // Ref isn't loaded. Ignore attempts to affect its selection state.
         //
         return;
      auto& list = this->state.selection.refs;
      auto  it   = std::find_if(list.begin(), list.end(), [&stub](const cobb::value_type_of<decltype(list)>& item) { return item.stub == &stub; });
      bool  has  = it != list.end();
      if (has == state)
         return;
      if (state) {
         this->_select_ref(*ref_info);
      } else {
         if constexpr (selection_vector_is_unordered) {
            cobb::unordered_erase(list, it);
         } else {
            list.erase(it);
         }
         this->_on_ref_deselected(stub);
      }
   }
   void core::toggleRefSelectionState(dovah::form_stub& stub) {
      auto* ref_info = this->_get_loaded_refr_info(stub);
      if (!ref_info)
         //
         // Ref isn't loaded. Ignore attempts to affect its selection state.
         //
         return;
      auto& list = this->state.selection.refs;
      auto  it   = std::find_if(list.begin(), list.end(), [&stub](const cobb::value_type_of<decltype(list)>& item) { return item.stub == &stub; });
      if (it == list.end()) {
         this->_select_ref(*ref_info);
      } else {
         if constexpr (selection_vector_is_unordered) {
            cobb::unordered_erase(list, it);
         } else {
            list.erase(it);
         }
         this->_on_ref_deselected(stub);
      }
   }
   void core::deselectAllRefs() {
      std::vector<dovah::form_stub*> deselected;
      //
      auto& list = this->state.selection.refs;
      auto  size = list.size();
      deselected.resize(size);
      for (size_t i = 0; i < size; ++i)
         deselected[i] = list[i].stub;
      list.clear();
      //
      for (auto* stub : deselected) {
         this->_on_ref_deselected(*stub);
      }
   }
   void core::replaceRefSelection(dovah::form_stub& stub) {
      this->deselectAllRefs();
      this->setRefSelectionState(stub, true);
   }
   void core::setCellGridSize(int size) {
      this->_resize_cell_grid(size);
   }

   void core::unloadAll() {
      this->_set_current_area_impl(nullptr);
   }
}