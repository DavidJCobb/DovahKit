#include "worldedit.h"
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
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/assets.h"
#include "helpers/qt/strings.h"
#include "nif/notice_code_t.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"
#include "vulkan/helpers/glm_transform_from_beth.h"
#include "vulkan/rendered_light.h"
#include "vulkan/surface_renderer.h"
#include "widgets/DKVulkanView.h"

namespace {
   static constexpr bool require_complete_implementation = false;
}

namespace {
   static constexpr float move_speed_normal = 180.0F;
   static constexpr float move_speed_mult_precision = 0.3F;
   static constexpr float move_speed_mult_boost     = 2.0F;

   static constexpr float turn_speed_per_second = glm::radians<float>(90);
}

namespace {
   bool _load_refr_model(nifDK::file& model, dovah::loaded_forms::components::model& src) {
      std::filesystem::path path = std::string("meshes") + (src.model_path[0] == '/' || src.model_path[0] == '\\' ? "" : "\\") + src.model_path;
      std::unique_ptr<dovah::bsa_archived_file> file(dovahkit::subsystems::assets::get().lookup_game_asset(path));
      if (!file) {
         qDebug("Failed to open NIF file: <%s>", path.string().c_str());
         return false;
      }
      model.read((void*)file->data(), file->size());
      //
      auto& error = model.read_error();
      if (error.code != nifDK::default_notice_code) {
         qDebug("Failed to parse NIF file: <%s>\n - Error code %08X.", path.string().c_str(), error.code);
         #if _DEBUG
            __debugbreak();
         #endif
         return false;
      }
      qDebug("NIF parsed. Passing to surface_renderer...");
      return true;
   }
}

namespace dovahkit::subsystems {
   worldedit::worldedit() : QObject(nullptr) {
      auto& core = DovahKitCore::get();
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
            if (form != this->loaded_cell.stub)
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

   void worldedit::_unload_refr(dovah::form_stub& stub) {
      if (!this->target_view)
         return;
      auto&  list = this->loaded_refs;
      size_t size = list.size();
      size_t i    = 0;
      for (; i < size; ++i)
         if (list[i].stub == &stub)
            break;
      if (i >= size)
         return;
      //
      auto* sr   = this->target_view->surfaceRenderer();
      auto& refr = list[i];
      if (auto& h = refr.vulkan_handles.light; !h.empty()) {
         h.destroy();
      }
      if (refr.nif)
         sr->remove_nif(*refr.nif);
      //
      list.erase(list.begin() + i);
   }
   void worldedit::_unload_cell(dovah::form_stub* cell) {
      if (!this->target_view)
         return;
      if (cell != this->loaded_cell.stub)
         return;
      //
      auto* sr   = this->target_view->surfaceRenderer();
      auto& list = this->loaded_refs;
      for (auto& refr : list) {
         auto* stub   = refr.stub;
         auto* parent = stub->get_parent_form();
         if (parent != cell)
            continue;
         //
         if (auto& h = refr.vulkan_handles.light; !h.empty()) {
            h.destroy();
         }
         if (refr.nif)
            sr->remove_nif(*refr.nif);
      }
      list.erase(
         std::remove_if(
            list.begin(),
            list.end(),
            [cell](refr& item) {
               return item.stub->get_parent_form() == cell;
            }
         ),
         list.end()
      );
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
      auto* model = new nifDK::file;
      if (!_load_refr_model(*model, *form_model)) {
         delete model;
         return false;
      }
      model->owning_form = &stub;
      //
      auto& item = this->loaded_refs.emplace_back();
      item.stub = &stub;
      item.form = stub.load().ptr_cast<dovah::loaded_forms::ObjectReference>();
      item.nif.reset(model);
      //
      out_pos = loaded->position;
      out_rot = loaded->rotation;
      out_is_coc = base->formID == dovah::hardcoded_form_ids::COCMarkerHeading;
      sr->add_nif(
         *model,
         glm::vec3{ loaded->position.x, loaded->position.y, loaded->position.z },
         glm::vec3{ loaded->rotation.x, loaded->rotation.y, loaded->rotation.z },
         scale
      );
      return true;
   }
   void worldedit::_load_cell(dovah::form_stub* cell, bool move_camera_to) {
      if (!this->target_view)
         return;
      assert(cell && cell->formType == dovah::form_type::cell);
      this->loaded_cell.stub = cell;
      //
      glm::vec3 centroid = { 0, 0, 0 };
      glm::vec3 coc_pos  = { 0, 0, 0 };
      glm::vec3 coc_rot  = { 0, 0, 0 };
      size_t refr_count = 0;
      bool   found_coc_marker = false;
      //
      auto* sr = this->target_view->surfaceRenderer();
      dovah::form_stub_helpers::for_each_child_form(cell, [this, sr, &found_coc_marker, &refr_count, &centroid, &coc_pos, &coc_rot](dovah::form_stub* stub) {
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
         if (!found_coc_marker) {
            if (is_coc) {
               found_coc_marker = true;
               coc_pos = { pos.x, pos.y, pos.z };
               coc_rot = { rot.x, rot.y, rot.z };
            } else {
               centroid += glm::vec3{ pos.x, pos.y, pos.z };
            }
         }
         return false;
      });
      if (move_camera_to) {
         if (found_coc_marker) {
            coc_pos.z += 160; // the CK uses a vertical offset as well
            sr->set_camera_position(coc_pos);
            //
            auto& scene = sr->scene;
            auto& camera = scene.camera;
            camera.pitch = coc_rot.x - glm::radians<float>(90);
            camera.roll  = coc_rot.y;
            camera.yaw   = coc_rot.z;
            scene.update_camera();
         } else {
            centroid /= refr_count;
            sr->set_camera_position(centroid);
         }
      }
      //
      // Cell lighting parameters:
      //
      {
         auto _to_vec = [](const dovah::loaded_forms::color_t& color) {
            return glm::vec3{ (float)color.r / 255.0F, (float)color.g / 255.0F, (float)color.b / 255.0F };
         };
         //
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
      }
   }

   void worldedit::set_current_cell(dovah::form_stub* cell) {
      if (this->loaded_cell.stub == cell)
         return;
      if (auto* prior = this->loaded_cell.stub)
         this->_unload_cell(prior);
      if (cell) {
         assert(cell->formType == dovah::form_type::cell && "Worldedit was asked to load a cell, but the provided form is not a cell.");
         this->_load_cell(cell, true);
      }
   }
   void worldedit::set_target_view(DKVulkanView& view) {
      if (this->target_view == &view)
         return;
      assert(this->target_view == nullptr);
      this->target_view = &view;
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
      //
      // Done processing all tools.
      //
   }
}