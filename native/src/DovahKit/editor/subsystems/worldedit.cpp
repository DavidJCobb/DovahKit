#include "worldedit.h"
#include <algorithm> // std::swap
#include "dk3D/tools/_results.h"
#include "dk3D/DK3DInputHandler.h"
#include "dovah/forms/factories/hardcoded.h"
#include "dovah/files/bsa/bsa_archived_file.h"
#include "dovah/form_stub_helpers.h"
#include "dovah/forms/Cell.h"
#include "dovah/forms/Form.h"
#include "dovah/forms/ObjectReference.h"
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

namespace {
   static constexpr bool require_complete_implementation = false;
}

namespace {
   static constexpr bool debug_log_area_load_unload = true;
   static constexpr bool debug_log_mesh_load_unload = false;
}

namespace {
   static constexpr auto max_selected_refr_count = vulkanDK::config::max_rendered_bounds;

   static constexpr float move_speed_normal = 180.0F;
   static constexpr float move_speed_mult_precision = 0.3F;
   static constexpr float move_speed_mult_boost     = 2.0F;

   static constexpr float turn_speed_per_second = glm::radians<float>(90);
}

namespace dovahkit::subsystems {
   #pragma region selected_refr_info
   worldedit::selected_refr_info::selected_refr_info() {}
   worldedit::selected_refr_info::~selected_refr_info() {
      this->handle.destroy();
   }

   worldedit::selected_refr_info& worldedit::selected_refr_info::operator=(selected_refr_info&& o) noexcept {
      std::swap(this->stub,   o.stub);
      std::swap(this->handle, o.handle);
      return *this;
   }
   #pragma endregion

   worldedit::worldedit() : QObject(nullptr) {
      this->loaded_cells.resize(5);

      auto& core = DovahKitCore::get();
      QObject::connect(&core, &DovahKitCore::dataAcquireComplete, this, &worldedit::_update_default_land_textures);
      QObject::connect(&core, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* form, bool just_flagging) {
         if (form->formType == dovah::form_type::cell) {
            this->_unload_cell(form);
            static_assert(!require_complete_implementation, "TODO: Interior cells: if the current cell is unloaded, reset lighting/fog params for the renderer.");
            return;
         }
         if (dovah::form_type_info::form_type_is_reference(form->formType)) {
            this->_unload_refr(*form);
            return;
         }
         if (dovah::form_type_info::form_type_is_base_form(form->formType)) {
            static_assert(!require_complete_implementation, "TODO: Find all loaded refs using this base form, and update them (show error NIF).");
            return;
         }
      });
      QObject::connect(&core, &DovahKitCore::formModified, this, [this](dovah::form_stub* form) {
         if (form->formType == dovah::form_type::cell) {
            if (!this->is_cell_loaded(form))
               return;
            static_assert(!require_complete_implementation, "TODO: Interior cells: check for changes to lighting params, and update Vulkan state.");
            static_assert(!require_complete_implementation, "TODO: Exterior cells: check for changes to region, water height, etc., and update as needed.");
            return;
         }
         if (dovah::form_type_info::form_type_is_reference(form->formType)) {
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: Check for changes to render-relevant REFR fields, and update as needed.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: If we're in an exterior and an unselected REFR is moved out of the loaded area, unload the REFR.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is loaded: If we're in an exterior and a selected REFR is moved to another world or an interior, unload the REFR.");
            static_assert(!require_complete_implementation, "TODO: If the REFR is NOT loaded: If we're in an exterior and the REFR is moved into the loaded area, load it.");
            return;
         }
         if (dovah::form_type_info::form_type_is_base_form(form->formType)) {
            static_assert(!require_complete_implementation, "TODO: Check if render-relevant properties (i.e. model; light data) have changed. If so, find all loaded refs using this base form, and update them.");
            return;
         }
      });
   }

   vulkanDK::rendered_bounds_handle worldedit::_make_bounds_for(const refr& item, bounds_source& src) {
      src = bounds_source::undefined;
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
         src = bounds_source::nif;
      } else {
         //
         // TODO: Pull OBND and use it.
         //
         bounds_min = { -128, -128, -128 };
         bounds_max = {  128,  128,  128 };
         src = bounds_source::none;
      }
      return sr->add_bounds(bounds_min, bounds_max, transform);
   }
   worldedit::refr* worldedit::_get_loaded_refr_info(const dovah::form_stub& stub) {
      for (auto& item : this->loaded_refs)
         if (item.stub == &stub)
            return &item;
      return nullptr;
   }
   worldedit::cell* worldedit::_get_loaded_cell_info(const dovah::form_stub& stub) {
      for (auto& item : this->loaded_cells)
         if (item.stub == &stub)
            return &item;
      return nullptr;
   }
   void worldedit::_unload_refr(refr& refr, bool handle_deselection) {
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
   void worldedit::_unload_refr(dovah::form_stub& stub) {
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
   void worldedit::_unload_cell(cell& loaded) {
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
   void worldedit::_unload_cell(dovah::form_stub* cell) {
      auto& list = this->loaded_cells;
      auto  it = std::find_if(list.begin(), list.end(), [cell](const worldedit::cell& item) { return item.stub == cell; });
      if (it == list.end())
         return;
      this->_unload_cell(*it);
      return;
   }
   void worldedit::_unload_all_cells() {
      for (auto& item : this->loaded_cells)
         this->_unload_cell(item);
   }
   bool worldedit::_load_refr(dovah::form_stub& stub, cobb::vector3<float>& out_pos, cobb::vector3<float>& out_rot, bool& out_is_coc) {
      auto* base = dovah::form_stub_helpers::get_base_form(&stub);
      if (!base)
         return false;
      //
      auto* sr = this->target_view->surfaceRenderer();
      if (base->formType == dovah::form_type::light) {
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
   void worldedit::_load_cell(dovah::form_stub* cell, loaded_cell_grid_coord gx, loaded_cell_grid_coord gy) {
      if (!this->target_view)
         return;
      assert(cell && cell->formType == dovah::form_type::cell);
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
      //
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
      dovah::form_stub_helpers::for_each_child_form(cell, [this, sr, &refr_count](dovah::form_stub* stub) {
         if (stub->formType != dovah::form_type::reference)
            return false;
         //
         cobb::vector3<float> pos;
         cobb::vector3<float> rot;
         bool is_coc;
         //
         if (!this->_load_refr(*stub, pos, rot, is_coc))
            return false;
         ++refr_count;
         //
         return false;
      });
      //
      // Signals:
      //
      emit this->cellLoaded(*cell);
   }

   void worldedit::_center_camera_on_cell(dovah::form_stub& stub) {
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
         sr->set_camera_position(coc_pos);
         //
         auto& scene = sr->scene;
         auto& camera = scene.camera;
         camera.pitch = coc_rot.x - glm::radians<float>(90);
         camera.roll  = 0.0;
         camera.yaw   = coc_rot.z;
         scene.update_camera();
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

   /*static*/ void worldedit::_on_renderer_nif_batch_loaded() {
      glm::vec3 bounds_min;
      glm::vec3 bounds_max;

      auto& self = worldedit::get();
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
   void worldedit::_on_renderer_attached() {
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
   }
   void worldedit::_on_renderer_loss_imminent(vulkanDK::surface_renderer& sr) {
      auto& hooks = sr.get_hooks();
      if (auto& hook = hooks.nif_batches.on_background_use_complete; hook == &_on_renderer_nif_batch_loaded)
         hook = nullptr;
   }
   void worldedit::_on_renderer_lost() {
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
   void worldedit::_update_default_land_textures() {
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
               diffuse_path = QLatin1Literal("textures/Landscape/") + diffuse_path;
         }
      }
      if (auto* setting = ini.setting("Landscape", "sDefaultLandNormalTexture")) {
         auto value = setting->currentValue();
         if (value.userType() == QMetaType::QString) {
            normals_path = value.toString();
            if (!normals_path.isEmpty())
               normals_path = QLatin1Literal("textures/Landscape/") + normals_path;
         }
      }
      //
      sr->set_default_land_textures(diffuse_path, normals_path);
   }
   
   void worldedit::_set_current_area_impl(dovah::form_stub* cell_or_world, int32_t grid_x, int32_t grid_y) {
      dovah::form_stub* cell  = nullptr;
      dovah::form_stub* world = nullptr;
      if (cell_or_world) {
         if (cell_or_world->formType == dovah::form_type::cell) {
            cell = cell_or_world;
            if (this->target_area.cell == cell)
               return;
            //
            world = cell->get_parent_form();
            if (world && world->formType != dovah::form_type::worldspace)
               world = nullptr;
         } else if (cell_or_world->formType == dovah::form_type::worldspace) {
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
               this->loaded_cells.shift_by(-diff_x, -diff_y, [this, world, &gp_now](worldedit::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Unloading cell at (%d, %d) due to set-current-area...", (gp_now.x + x), (gp_now.y + y));
                  }
                  this->_unload_cell(data);
               });
            }
            this->loaded_cells.for_each([this, world, &gp_now](worldedit::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
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
            assert(cell->formType == dovah::form_type::cell && "Worldedit was asked to load a cell, but the provided form is not a cell.");
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
                  sgs.ambient_light_color = _to_vec(lt.ambient);
                  sgs.sun_color      = _to_vec(lt.directional);
                  static_assert(!require_complete_implementation, "TODO: sgs.sun_dir");
                  // TODO: sgs.sun_dir
                  sgs.fog_color_near = _to_vec(lt.fog_color_near);
                  sgs.fog_color_far  = _to_vec(lt.fog_color_far);
                  sgs.fog_plane_near = lt.fog_distance_near;
                  sgs.fog_plane_far  = lt.fog_distance_far;
                  sgs.fog_power      = lt.fog_power;
                  sgs.fog_max        = lt.fog_max;
                  sgs.interior_clip_distance = lt.fog_distance_clip;
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
            }
         }
      }
      //
      // Done!
      //
      emit this->currentAreaChanged(world ? world : cell);
   }

   void worldedit::_resize_cell_grid(size_t length) {
      size_t prior_length = this->loaded_cells.length();
      if (length == prior_length)
         return;
      if (!this->target_area.world) {
         //
         // Fast path when viewing interiors:
         //
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
            this->loaded_cells.for_each([this, world, &gp_now](worldedit::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
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

   void worldedit::center_on_refr(dovah::form_stub& ref) {
      auto* cell = ref.get_parent_form();
      if (!cell || cell->formType != dovah::form_type::cell)
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
               sr->set_camera_position(pos);
               //
               auto& scene  = sr->scene;
               auto& camera = scene.camera;
               camera.pitch = glm::radians<float>(-90);
               camera.roll  = 0.0;
               camera.yaw   = loaded->rotation.z;
               scene.update_camera();
            }
         }
      }
   }
   void worldedit::set_current_area(dovah::form_stub* cell_or_world) {
      this->_set_current_area_impl(cell_or_world);
   }
   void worldedit::set_current_area(dovah::form_stub* world, int32_t grid_x, int32_t grid_y) {
      if (world && world->formType != dovah::form_type::worldspace) {
         #if _DEBUG
            __debugbreak(); // invalid argument
         #endif
         return;
      }
      this->_set_current_area_impl(world, grid_x, grid_y);
   }
   void worldedit::set_target_view(DKVulkanView& view) {
      if (this->target_view == &view)
         return;
      assert(this->target_view == nullptr);
      this->target_view = &view;
      //
      DK3DInputHandler::get().setTargetView(&view);
      //
      QObject::connect(&view, &DKVulkanView::rendererReady, this, &worldedit::_on_renderer_attached, Qt::UniqueConnection);
      if (auto* sr = view.surfaceRenderer()) {
         //
         // Renderer is already ready.
         //
         this->_on_renderer_attached();
      }
      //
      QObject::connect(&view, &DKVulkanView::rendererErrorKillImminent, this, &worldedit::_on_renderer_loss_imminent);
      QObject::connect(&view, &DKVulkanView::rendererKilledDueToError, this, &worldedit::_on_renderer_lost);
      QObject::connect(&view, &QObject::destroyed, this, [this]() {
         DK3DInputHandler::get().setTargetView(nullptr);
         this->target_view = nullptr;
         //
         this->_on_renderer_lost();
      });
      //
      QObject::connect(&view, &DKVulkanView::renderedMeshClicked, this, [this](vulkanDK::rendered_mesh_handle handle) {
         assert(!handle.empty());
         auto* nif = handle->owning_nif;
         if (!nif)
            return;
         auto* stub = nif->owning_form;
         if (!stub)
            return;
         auto* base = dovah::form_stub_helpers::get_base_form(stub);
         emit this->statusBarMessage(
            QString("Clicked on form %1 (base %2).")
               .arg(editor_helpers::form_identifiers_to_string(stub))
               .arg(editor_helpers::form_identifiers_to_string(base)),
            3000
         );
      });
   }

   void worldedit::view_input_poll_handler(DKVulkanView& view) {
      if (&view != this->target_view)
         return;
      if (!view.isListeningForInput())
         return;
      DK3D::combined_tool_results results;
      double delta;
      DK3DInputHandler::get().update(results, delta);
      //
      auto* sr = view.surfaceRenderer();
      if (!sr)
         //
         // Don't execute commands "blind." If there's no renderer, exit.
         //
         return;
      //
      #pragma region modify_camera_speed_flags
      //
      // This must run before we apply camera movements.
      //
      {
         const auto& data = results.get_member<DK3D::tools::modify_camera_speed_flags>();
         auto& mask = this->state.camera_speed;
         {
            constexpr auto flag = camera_speed_flags::boost;
            switch (data.boost) {
               using enum DK3D::bool_operation;
               case set_true:
                  mask.set<flag>();
                  break;
               case set_false:
                  mask.reset<flag>();
                  break;
               case invert:
                  mask.flip<flag>();
                  break;
            }
         }
         {
            constexpr auto flag = camera_speed_flags::precision;
            switch (data.precision) {
               using enum DK3D::bool_operation;
               case set_true:
                  mask.set<flag>();
                  break;
               case set_false:
                  mask.reset<flag>();
                  break;
               case invert:
                  mask.flip<flag>();
                  break;
            }
         }
      }
      #pragma endregion
      #pragma region move_camera and turn_camera
      {
         DKVulkanCameraUpdate update;
         update.delta_seconds = delta;
         {
            const auto& data = results.get_member<DK3D::tools::move_camera>();
            update.move.direction      = { data.x, data.y, data.z };
            update.move.scale_by_delta = false;
            //
            // Results from non-tap binds (e.g. "while" binds, scalars, vectors) get scaled by the 
            // delta in the code above. This means that we need to turn off scaling in this particular 
            // step here.
            //
            update.move.speed = move_speed_normal * delta; // must specify this (as speed * elapsed) rather than relying on direction alone, because the direction vector gets normalized when we pass it in
            if (this->state.camera_speed.test<camera_speed_flags::boost>())
               update.move.speed *= move_speed_mult_boost;
            if (this->state.camera_speed.test<camera_speed_flags::precision>())
               update.move.speed *= move_speed_mult_precision;
         }
         {
            const auto& data = results.get_member<DK3D::tools::turn_camera>();
            update.turn.roll  = data.roll;
            update.turn.pitch = data.pitch;
            update.turn.yaw   = data.yaw;
            update.turn.scale_by_delta = false;
            //
            update.turn.speed = turn_speed_per_second;
         }
         sr->scene.adjust_camera(update);
      }
      #pragma endregion
      #pragma region attempt_on_screen_selection
      {
         const auto& data = results.get_member<DK3D::tools::attempt_on_screen_selection>();
         if (data.sweep) {
            //
            // TODO
            //
            static_assert(!require_complete_implementation, "TODO: Only modify an entity's selection state on the first frame the cursor sweeps over it.");
         } else {
            if (data.position == DK3D::pointer_position_type::mouse) {
               auto handle = sr->rendered_mesh_at(data.mouse.x(), data.mouse.y());
               if (!handle.empty()) {
                  if (auto* nif = handle->owning_nif) {
                     if (auto* stub = nif->owning_form) {
                        switch (data.operation) {
                           case DK3D::selection_operation::no_op:
                              break;
                           case DK3D::selection_operation::toggle:
                              this->toggleRefSelectionState(*stub);
                              break;
                           case DK3D::selection_operation::add:
                           case DK3D::selection_operation::remove:
                              this->setRefSelectionState(*stub, data.operation == DK3D::selection_operation::add);
                              break;
                           case DK3D::selection_operation::replace:
                              this->replaceRefSelection(*stub);
                              break;
                        }
                     }
                  }
               }
            } else {
               //
               // TODO
               //
               static_assert(!require_complete_implementation, "TODO: Support performing a selection at the reticle.");
            }
         }
      }
      #pragma endregion
      #pragma region debug_dump_landscape_details
      if (sr) {
         const auto& data = results.get_member<DK3D::tools::debug_dump_landscape_details>();
         if (data.exists && data.position == DK3D::pointer_position_type::mouse) {
            vulkanDK::raycast rc(*sr);
            rc.set_screen_relative_raycast(data.mouse.x(), data.mouse.y());

            sr->do_raycast(rc);
            if (rc.result.hit) {
               if (std::holds_alternative<vulkanDK::rendered_landscape_handle>(rc.result.entity)) {
                  auto handle = std::get<vulkanDK::rendered_landscape_handle>(rc.result.entity);
                  auto pos    = rc.result.hit.position - handle->frame_drawing_data.position;
                  
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
                     vulkanDK::vertex_landscape* vert = nullptr;

                     #if _DEBUG
                        //
                        // TODO: console-print the vert attributes
                        //
                        __debugbreak();
                     #endif

                  }
               }
            }
         } else {
            //
            // TODO
            //
            static_assert(!require_complete_implementation, "TODO: Support performing a query at the reticle.");
         }
      }
      #pragma endregion
      //
      // Done processing all tools.
      //
      if (auto* world = this->target_area.world) {
         //
         // Process (un)loading cells as the camera moves.
         //
         const auto& camera_pos = sr->scene.camera.position;
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
               this->loaded_cells.shift_by(-diff_x, -diff_y, [this, world, cgx, cgy](worldedit::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
                  if constexpr (debug_log_area_load_unload) {
                     qDebug("[Worldedit] Unloading cell at (%d, %d) due to camera movement...", (cgx + x), (cgy + y));
                  }
                  this->_unload_cell(data.stub);
               });
               this->loaded_cells.for_each([this, world, cgx, cgy](worldedit::cell& data, loaded_cell_grid_coord x, loaded_cell_grid_coord y) {
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

   bool worldedit::is_cell_loaded(const dovah::form_stub* cell) const {
      if (!cell)
         return false;
      for (const auto& item : this->loaded_cells)
         if (item.stub == cell)
            return true;
      return false;
   }
   bool worldedit::is_current_cell(const dovah::form_stub* cell) const {
      if (!cell)
         return false;
      if (this->target_area.world) {
         return cell == this->loaded_cells.at(0, 0).stub;
      }
      return cell == this->target_area.cell;
   }
   bool worldedit::is_ref_loaded(const dovah::form_stub* ref) const {
      if (!ref)
         return false;
      for (const auto& item : this->loaded_refs)
         if (item.stub == ref)
            return true;
      return false;
   }
   bool worldedit::is_ref_selected(const dovah::form_stub* ref) const {
      for (auto& item : this->state.selection.refs)
         if (item.stub == ref)
            return true;
      return false;
   }

   void worldedit::setRefSelectionState(dovah::form_stub& stub, bool state) {
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
         if (list.size() >= max_selected_refr_count) {
            return;
         }
         bounds_source source_info;
         list.push_back({ &stub, _make_bounds_for(*ref_info, source_info) });
         if (source_info != bounds_source::undefined && source_info != bounds_source::nif) {
            list.back().update_on_nif_load = true;
         }
         emit this->refSelected(stub);
         emit this->refSelectionChanged(stub, true);
      } else {
         list.erase(it);
         emit this->refDeselected(stub);
         emit this->refSelectionChanged(stub, false);
      }
   }
   void worldedit::toggleRefSelectionState(dovah::form_stub& stub) {
      auto* ref_info = this->_get_loaded_refr_info(stub);
      if (!ref_info)
         //
         // Ref isn't loaded. Ignore attempts to affect its selection state.
         //
         return;
      auto& list = this->state.selection.refs;
      auto  it   = std::find_if(list.begin(), list.end(), [&stub](const cobb::value_type_of<decltype(list)>& item) { return item.stub == &stub; });
      if (it == list.end()) {
         bounds_source source_info;
         list.push_back({ &stub, _make_bounds_for(*ref_info, source_info) });
         if (source_info != bounds_source::undefined && source_info != bounds_source::nif) {
            list.back().update_on_nif_load = true;
         }
         emit this->refSelected(stub);
         emit this->refSelectionChanged(stub, true);
      } else {
         list.erase(it);
         emit this->refDeselected(stub);
         emit this->refSelectionChanged(stub, false);
      }
   }
   void worldedit::deselectAllRefs() {
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
         emit this->refDeselected(*stub);
         emit this->refSelectionChanged(*stub, false);
      }
   }
   void worldedit::replaceRefSelection(dovah::form_stub& stub) {
      this->deselectAllRefs();
      this->setRefSelectionState(stub, true);
   }
   void worldedit::setCellGridSize(int size) {
      this->_resize_cell_grid(size);
   }
}