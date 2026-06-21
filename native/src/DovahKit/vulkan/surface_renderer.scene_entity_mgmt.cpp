#include "surface_renderer.h"
#pragma region Includes

   #include "./exceptions.h"

   #pragma region Entity types
      #include "./rendered_landscape.h"
      #include "./rendered_light.h"
      #include "./rendered_nif.h"
   #pragma endregion

   #pragma region Includes shared by multiple entity types
      #include <QDir> // for normalizing asset paths

      #include "dovah/forms/ObjectReference.h"

      #include "./helpers/glm_transform_from_beth.h"

      #pragma region Lifetime
         #include "./asset_loading/reserve_meshes_for_nif.h"
         #include "./asset_loading/worker_thread_for_meshes.h"
         #include "./asset_loading/worker_thread_for_nifs.h"
         #include "./asset_loading/worker_thread_for_textures.h"
         #include "./scene_entities/owned_gpu_resource_upload_operation.h"
      #pragma endregion
   #pragma endregion

   #pragma region Includes for loaded_texture
      #include "./dds/texture.h"
   #pragma endregion

   #pragma region Includes for rendered_landscape
      #include "dovah/forms/Landscape.h"
      #include "dovah/forms/LandTexture.h"
      #include "dovah/forms/TextureSet.h"
   #pragma endregion

   #pragma region Includes for rendered_light
      #include "dovah/forms/components/extra_data/types/l/light.h"
      #include "dovah/forms/components/extra_data/types/r/radius.h"
      #include "dovah/forms/Light.h"
   #pragma endregion

   #pragma region Includes for rendered_nif
      #include "nif/blocks/_DKVulkanInterface.h"
      #include "nif/block.h"
   #pragma endregion

   // debug: loading arbitrary files from disk for use as textures
   #include <QBuffer>
   #include <QFile>
   #include <QImageReader>

   // surface_renderer::add_dds_texture (i.e. synchronous texture loads)
   #include "dovah/files/bsa/bsa_archived_file.h"
   #include "editor/subsystems/assets.h"
#pragma endregion

namespace {
   static constexpr const bool debug_log_scene_object_lifetimes = false;
}

namespace vulkanDK {
   #pragma region loaded_texture
   size_t surface_renderer::add_dds_texture(QString texture_path) {
      constexpr size_t fail = scene::index_of_none;
      //
      if (!texture_path.endsWith(".dds", Qt::CaseInsensitive))
         return fail;
      texture_path = QDir::cleanPath(texture_path).toLower();
      //
      {
         size_t i = this->scene.reuse_scene_texture(texture_path);
         if (i != scene::index_of_none) {
            if constexpr (debug_log_scene_object_lifetimes) {
               qDebug("[vulkanDK::scene_renderer::add_texture] Reusing texture index %u for texture path <%s>", i, qUtf8Printable(texture_path));
            }
            return i;
         }
      }
      //
      std::unique_ptr<dovah::bsa_archived_file> file;
      {
         if (!texture_path.startsWith("textures/")) {
            qDebug("[vulkanDK::surface_renderer::add_dds_texture] Invalid texture path (doesn't start with textures folder): %s", qUtf8Printable(texture_path));
            return fail;
         }
         std::filesystem::path path = texture_path.toStdWString();
         file.reset(dovahkit::subsystems::assets::get_or_create().lookup_game_asset(path)); // TODO: switch to `get` once we're sure the asset subsystem is constructed elsewhere
         if (!file) {
            qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to open texture: %s", qUtf8Printable(texture_path));
            return fail;
         }
      }
      dds::texture tex;
      tex.data = file->data();
      tex.size = file->size();
      //
      if (!tex.read()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] Failed to read DDS header: %s", qUtf8Printable(texture_path));
         return fail;
      }
      if (!tex.pixel_data() || !tex.pixel_data_size()) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] No DDS data available: %s", qUtf8Printable(texture_path));
         return fail;
      }
      const auto vulkan_metadata = image_metadata::from_dds_header(tex.metadata, tex.pixel_data_size());
      if (vulkan_metadata.format == VkFormat::VK_FORMAT_UNDEFINED) {
         qDebug("[vulkanDK::surface_renderer::add_dds_texture] DDS texture format did not map to Vulkan: %s", qUtf8Printable(texture_path));
         return fail;
      }
      //
      // Create scene texture.
      //
      auto texture_index = this->scene.insert_new_scene_entity<loaded_texture>();
      if (texture_index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_dds_texture] Cannot add new rendered textures. Maximum has been reached.");
         return fail;
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_dds_texture] Creating new texture at index %u for texture path <%s>", texture_index, qUtf8Printable(texture_path));
      }
      auto& target = this->scene.entities_of_type<loaded_texture>()[texture_index];
      target.w    = tex.metadata.width;
      target.h    = tex.metadata.height;
      target.path = texture_path;
      //
      // Queue transfer to the GPU:
      // 
      // Right now, `tex` is borrowing a buffer directly from the BSA-archived file, and that 
      // buffer's gonna get deleted when we're done with the BSA data. We can't simply "steal" 
      // it from the BSA-archived file object, because it may actually be shared with the BSA 
      // itself (i.e. if the file is uncompressed). We have to instead just copy the buffer.
      //
      auto* copy = malloc(tex.size);
      memcpy(copy, tex.data, tex.size);
      tex.data = copy;
      //
      target.lifetime.life_state = scene_entities::life_state::active_pending_upload;
      target.prepare_for_gpu_upload(vulkan_metadata, std::move(tex));
      ++this->uploading.pending_upload_counts.value_for<loaded_texture>();
      //
      // And now we're done!
      //
      return texture_index;
   }
   void surface_renderer::lookup_or_reserve_dds_texture(QString texture_path, size_t& texture_entity_index, bool& already_existed) {
      //
      // This function is used to support multi-threaded loading of NIFs and their referenced 
      // textures. If we want to load both NIFs and textures in a multithreaded manner, then 
      // it is easiest to coordinate loading if we behave as follows:
      // 
      //  - If the texture file doesn't exist or does not use the DDS extension, then fail 
      //    immediately and return index-of-none.
      // 
      //  - If the texture file is already loaded, return its index.
      // 
      //  - If we have reached the max texture count, return index-of-none.
      // 
      //  - Otherwise, reserve a new texture index and return it. Even if the DDS file later 
      //    fails to load (e.g. invalid file data), this texture index will remain in use 
      //    until all referencing meshes are gone.
      // 
      // That way, we don't need to handle complex interdependencies between resources that 
      // are loaded on different threads, and whatnot.
      //
      already_existed = false;
      if (!texture_path.endsWith(".dds", Qt::CaseInsensitive)) {
         texture_entity_index = scene::index_of_none;
         return;
      }
      texture_path = QDir::cleanPath(texture_path).toLower();
      //
      // Check for an already-existing texture:
      //
      texture_entity_index = this->scene.reuse_scene_texture(texture_path);
      if (texture_entity_index != scene::index_of_none) {
         if constexpr (debug_log_scene_object_lifetimes) {
            qDebug("[vulkanDK::scene_renderer::lookup_or_reserve_dds_texture] Reusing texture index %u for texture path <%s>", texture_entity_index, qUtf8Printable(texture_path));
         }
         already_existed = true;
         return;
      }
      //
      // Reserve a new texture, and queue it for multi-threaded loading.
      //
      texture_entity_index = this->scene.insert_new_scene_entity<loaded_texture>();
      if (texture_entity_index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::lookup_or_reserve_dds_texture] Cannot add new rendered textures. Maximum has been reached.");
         return;
      }
      auto& target = this->scene.entities_of_type<loaded_texture>()[texture_entity_index];
      target.path = texture_path;
      target.lifetime.life_state = scene_entities::life_state::active_background_loading;
      //
      this->loading.textures.enqueue(texture_entity_index);
   }
   #pragma endregion

   #pragma region rendered_bounds
   rendered_bounds_handle surface_renderer::add_bounds(const glm::vec3& min, const glm::vec3& max, const glm::mat4& pivot_transform) {
      size_t index = this->scene.insert_new_scene_entity<rendered_bounds>();
      if (index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_bounds] Cannot add new rendered_bounds; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_bounds] Creating new bound at index %u.", index);
      }
      auto& item = this->scene.entities_of_type<rendered_bounds>()[index];
      item.set_size_and_transform(min, max, pivot_transform);
      
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_bounds>();
      
      return rendered_bounds_handle(*this, index);
   }
   void surface_renderer::remove_bounds(size_t i) {
      auto& list = this->scene.entities_of_type<rendered_bounds>();
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_bounds] Marking scene bounds %u for delete.", i);
      }
      ++this->scene.entities.pending_deletion_counts.value_for<rendered_bounds>();
      auto& item = list[i];
      item.mark_for_delete();
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_bounds>();
   }
   #pragma endregion

   #pragma region rendered_landscape
   rendered_landscape_handle surface_renderer::add_landscape(const glm::vec3& position) {
      size_t index = this->scene.insert_new_scene_entity<rendered_landscape>();
      if (index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_landscape] Cannot add new rendered_landscape; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_landscape] Creating new landscape at index %u.", index);
      }
      auto& item = this->scene.entities_of_type<rendered_landscape>()[index];
      item.set_position(position);
      
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_landscape>();
      
      return rendered_landscape_handle(*this, index);
   }
   rendered_landscape_handle surface_renderer::add_landscape(const glm::vec3& position, const dovah::loaded_forms::Landscape& land) {
      auto handle = this->add_landscape(position);
      if (handle.empty())
         return handle;
      //
      auto _load_textures = [this](loaded_texture_index& diffuse, loaded_texture_index& normals, dovah::form_stub& ltex) {
         auto  load = ltex.load().ptr_cast<dovah::loaded_forms::LandTexture>();
         if (!load)
            return;
         auto* txst = load->texture_set.get_form_stub();
         if (!txst)
            return;
         auto  txld = txst->load().ptr_cast<dovah::loaded_forms::TextureSet>();
         if (!txld)
            return;
         //
         diffuse.set(*this, this->add_dds_texture(QString("textures/") + txld->textures.diffuse.c_str()));
         normals.set(*this, this->add_dds_texture(QString("textures/") + txld->textures.normal.c_str()));
      };
      //
      auto& sp = handle->frame_drawing_data;
      for (size_t i = 0; i < 4; ++i) {
         sp.diffuse_base[i].clear(*this);
         sp.normals_base[i].clear(*this);
         for (size_t j = 0; j < rendered_landscape::max_usable_layers_per_quad; ++j) {
            sp.diffuse_blends.by_quad[i][j].clear(*this);
            sp.normals_blends.by_quad[i][j].clear(*this);
         }
         //
         auto* ltex = land.default_quad_textures[i].get_form_stub();
         if (ltex) {
            _load_textures(sp.diffuse_base[i], sp.normals_base[i], *ltex);
         }
         //
         auto& blends = land.alpha_layers_by_quad[i];
         for (auto& blend : blends) {
            if (blend.layer < 0 || blend.layer >= rendered_landscape::max_usable_layers_per_quad)
               continue;
            auto* ltex = blend.texture.get_form_stub();
            if (!ltex)
               continue;
            _load_textures(sp.diffuse_blends.by_quad[i][blend.layer], sp.normals_blends.by_quad[i][blend.layer], *ltex);
         }
      }
      //
      handle->import_vertex_data_from_form(land);
      return handle;
   }
   void surface_renderer::remove_landscape(size_t i) {
      auto& list = this->scene.entities_of_type<rendered_landscape>();
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_landscape] Marking scene landscape %u for delete.", i);
      }
      ++this->scene.entities.pending_deletion_counts.value_for<rendered_landscape>();
      auto& item = list[i];
      item.mark_for_delete();
      {
         for (auto& ti : item.frame_drawing_data.diffuse_base) {
            ti.clear(*this);
         }
         for (auto& ti : item.frame_drawing_data.diffuse_blends.all) {
            ti.clear(*this);
         }
         for (auto& ti : item.frame_drawing_data.normals_base) {
            ti.clear(*this);
         }
         for (auto& ti : item.frame_drawing_data.normals_blends.all) {
            ti.clear(*this);
         }
      }
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_landscape>();
   }
   #pragma endregion

   #pragma region rendered_light
   rendered_light_handle surface_renderer::add_light(dovah::loaded_forms::ObjectReference& refr) {
      auto* base = refr.base_form.get_form_stub();
      if (!base || base->form_type != dovah::form_type::light)
         return {};
      auto loaded_base = base->load().ptr_cast<dovah::loaded_forms::Light>();
      if (!loaded_base)
         return {};
      //
      rendered_light::frame_drawing_data_type params = {
         .transform = glm_transform_from_beth(refr.position, refr.rotation, 1.0F),
         .color     = {
            (float)loaded_base->color.r / 255.0,
            (float)loaded_base->color.g / 255.0,
            (float)loaded_base->color.b / 255.0,
         },
         .fade    = loaded_base->fade,
         .falloff = loaded_base->falloff_exponent,
         .fov     = loaded_base->fov,
         .radius  = (float)loaded_base->radius,
      };
      if (auto* ex = refr.extra_data.get<dovah::loaded_forms::components::extra_data_types::light>()) {
         params.fade += ex->fade;
         params.fov  += ex->fov;
      }
      if (auto* ex = refr.extra_data.get<dovah::loaded_forms::components::extra_data_types::radius>()) {
         params.radius += ex->value;
      }
      //
      params.type = rendered_light::light_type::omni;
      {
         switch (loaded_base->light_type) {
            using enum dovah::loaded_forms::Light::emitter_type;
            case omni:
               params.type = rendered_light::light_type::omni;
               break;
            case omni_shadow: // NOTE: we don't yet support shadowing, nor shadowed point lights
               params.type = rendered_light::light_type::omni_shadow;
               break;
            case hemi_shadow:
               params.type = rendered_light::light_type::hemi_shadow;
               break;
            case spot: // Bethesda's spot lights always cast shadows
            case spot_shadow:
               params.type = rendered_light::light_type::spot_shadow;
               break;
            default:
               return {}; // unsupported light type
         }
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_light] Attempting to spawn new light for [REFR:%08X] with base [LIGH:%08X]%s...", refr.stub.formID, base->formID, base->get_editor_id());
      }
      return this->add_light(params);
   }
   rendered_light_handle surface_renderer::add_light(const rendered_light::frame_drawing_data_type& in) {
      size_t light_index = this->scene.insert_new_scene_entity<rendered_light>();
      if (light_index == scene::index_of_none) {
         qDebug("[vulkanDK::scene_renderer::add_light] Cannot add new rendered_light; scene limits reached.");
         return {};
      }
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::add_light] Creating new light at index %u.", light_index);
      }
      auto& light = this->scene.entities_of_type<rendered_light>()[light_index];
      light.frame_drawing_data = in;
      light.set_transform(in.transform); // so that transform_inv is valid
      //
      if (light.can_cast_shadows()) {
         this->scene.mark_light_shadows_dirty();
      }
      //
      return rendered_light_handle(*this, light_index);
   }
   void surface_renderer::remove_light(size_t i) {
      auto& list = this->scene.entities_of_type<rendered_light>();
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_light] Marking scene light %u for delete.", i);
      }
      ++this->scene.entities.pending_deletion_counts.value_for<rendered_light>();
      auto& item = list[i];
      if (item.can_cast_shadows()) {
         this->scene.mark_light_shadows_dirty();
      }
      item.mark_for_delete();
   }
   #pragma endregion

   #pragma region rendered_mesh
   void surface_renderer::remove_mesh(size_t i) {
      auto& list = this->scene.entities_of_type<rendered_mesh>();
      if (i >= list.size())
         return;
      if constexpr (debug_log_scene_object_lifetimes) {
         qDebug("[vulkanDK::scene_renderer::remove_mesh] Marking scene mesh %u for delete.", i);
      }
      ++this->scene.entities.pending_deletion_counts.value_for<rendered_mesh>();
      auto& item = list[i];
      item.mark_for_delete();
      if (item.owning_nif) {
         item.owning_nif->sever_connection_to(rendered_mesh_handle(*this, i));
         item.owning_nif = nullptr;
      }
      {
         auto& list = this->scene.entities_of_type<loaded_texture>();
         for (auto& ti : item.texture_indices.list) {
            if (ti.empty())
               continue;
            ti.clear(*this);
         }
      }
      //
      for (auto& fif : this->swap_chain.frames_in_flight)
         fif.on_scene_entity_added_or_removed<rendered_mesh>();
   }
   #pragma endregion

   #pragma region rendered_nif
   rendered_nif* surface_renderer::add_nif(
      dovah::form_stub& stub,
      dovah::loaded_forms::components::model& model,
      const glm::vec3& pos,
      const glm::vec3& rot,
      float scale
   ) {
      auto* nif = new rendered_nif;
      {
         nif->multi_thread_state.manager = this;
         nif->multi_thread_state.flags |= rendered_nif::loading_flag::load_queued;

         this->loading.meshes.enqueue(asset_loading::queued_nif_load{
            .form_data = {
               .loaded_form = &stub,
               .model = &model,
            },
            .nif = nif,
            .transform = glm_transform_from_beth(pos, rot, scale),
         });
      }
      return nif;
   }
   void surface_renderer::remove_nif(rendered_nif& model) {
      bool any_removed = false;
      for (auto* block : model.all_blocks) {
         auto* intfc = dynamic_cast<nifDK::block_interfaces::_DKVulkanMeshInterface*>(block);
         if (!intfc)
            continue;
         auto& handle = intfc->vulkan_state.mesh_handle;
         if (!handle.empty()) {
            if (handle.renderer() != this) {
               qDebug("[surface_renderer::remove_nif] WARNING: Geometry object belongs to a different renderer!");
               return;
            }
            handle.destroy();
            any_removed = true;
         }
      }
      if (any_removed) {
         for (auto& fif : this->swap_chain.frames_in_flight)
            fif.on_scene_entity_added_or_removed<rendered_mesh>();
      }
   }
   #pragma endregion

   #pragma region Entity lifetime management
   void surface_renderer::_execute_asset_multithreaded_load() {
      using texture_worker_t = asset_loading::worker_thread_for_textures;

      bool any_meshes   = !this->loading.meshes.empty();
      bool any_textures = !this->loading.textures.empty();
      if (!any_meshes && !any_textures) {
         return;
      }

      using maybe_textures = std::array<std::optional<texture_worker_t>, config::asset_loading_thread_count>;

      maybe_textures texture_workers;
      if (any_textures) {
         //
         // Textures have been queued to load with no associated NIF. Let's deal with those 
         // before we load any NIFs (since those will use the same queues as these textures).
         //
         for(size_t i = 0; i < texture_workers.size(); ++i) {
            auto& worker = texture_workers[i];
            worker.emplace(*this, i).start();
         }
         //
         for (auto& worker : texture_workers) {
            worker.value().wait();
            worker.reset();
         }
         //
         this->loading.textures.clear();
         //
         for (auto& fif : this->swap_chain.frames_in_flight) {
            fif.on_scene_entity_added_or_removed<loaded_texture>();
         }
         any_textures = false;
      }
      if (any_meshes) {
         {
            using worker_t = asset_loading::worker_thread_for_nifs;
            std::array<worker_t, config::asset_loading_thread_count> workers = {
               worker_t{*this, 0},
               worker_t{*this, 1},
               worker_t{*this, 2},
               worker_t{*this, 3},
            };
            for (auto& worker : workers)
               worker.start();
            for (size_t i = 0; i < workers.size(); ++i) {
               workers[i].wait();
               //
               // When NIFs are loaded, we need to apply texture swaps (when necessary) and create 
               // `rendered_mesh`es and `loaded_texture`s for them. We'll load the contents of those 
               // scene entities later.
               //
               bool  any_succeeded = false;
               auto& list = this->loading.meshes.batches[i];
               for (auto& item : list) {
                  if (!item.nif->did_load_succeed()) {
                     continue;
                  }
                  any_succeeded = true;
                  //
                  auto& model = *item.form_data.model;
                  if (auto* casted = model.as_model_ts()) {
                     item.nif->apply_texture_swaps(*casted);
                  }
                  item.form_data.loaded_form = nullptr; // allow the form to unload, if nothing else is using it
                  item.nif->multi_thread_state.flags |= rendered_nif::loading_flag::generating_meshes;
                  asset_loading::reserve_meshes_for_nif(*this, *item.nif);
               }
               if (any_succeeded) {
                  if (this->hooks.nif_batches.on_background_loaded) {
                     (this->hooks.nif_batches.on_background_loaded)();
                  }
               }
            }
         }
         any_textures = !this->loading.textures.empty(); // texture loads may have been queued while loading NIFs
         if (any_textures) {
            for (size_t i = 0; i < texture_workers.size(); ++i) {
               auto& worker = texture_workers[i];
               worker.emplace(*this, i).start();
            }
         }
         {
            using worker_t = asset_loading::worker_thread_for_meshes;
            std::array<worker_t, config::asset_loading_thread_count> workers = {
               worker_t{*this, 0},
               worker_t{*this, 1},
               worker_t{*this, 2},
               worker_t{*this, 3},
            };
            for (auto& worker : workers)
               worker.start();
            for (auto& worker : workers)
               worker.wait();
         }
         //
         this->loading.meshes.for_each_queue_item([this](auto& item) {
            auto& mts = item.nif->multi_thread_state;
            mts.flags &= ~rendered_nif::loading_flag::is_in_background_use;
            mts.manager = nullptr;
         });
         this->loading.meshes.clear();
         //
         if (any_textures) {
            for (auto& worker : texture_workers)
               worker.value().wait();
            //
            this->loading.textures.clear();
         }
      }
      for (auto& fif : this->swap_chain.frames_in_flight) {
         if (any_textures)
            fif.on_scene_entity_added_or_removed<loaded_texture>();
         if (any_meshes)
            fif.on_scene_entity_added_or_removed<rendered_mesh>();
      }
   }

   bool surface_renderer::_execute_pending_scene_entity_gpu_uploads() {
      VkDeviceSize total_staging_size = 0;
      VkDeviceSize max_staging_size   = this->max_upload_buffer_size();

      this->uploading.pending_upload_counts.for_each([this, &total_staging_size, max_staging_size]<typename Entity>(const size_t pending_upload_count) {
         if (pending_upload_count <= 0)
            return;
         if (total_staging_size >= max_staging_size)
            return;
         auto& list = this->scene.entities_of_type<Entity>();
         for (const auto& entity : list) {
            if (entity.lifetime.life_state != scene_entities::life_state::active_pending_upload)
               continue;
            auto size  = entity.owned_gpu_resources_size();
            total_staging_size += size;
            auto align = entity.owned_gpu_resource_upload_alignment();
            if (auto misalign = total_staging_size % align; misalign) {
               total_staging_size += (align - misalign);
            }
            if (total_staging_size >= max_staging_size)
               break;
         }
      });

      if (total_staging_size <= 0)
         return false;

      auto& staging = this->uploading.staging;
      staging = this->create_staging_buffer(total_staging_size);

      auto& commands = this->uploading.commands;
      commands.top_level_begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

      auto upload = scene_entities::owned_gpu_resource_upload_operation(*this);
      upload.set_max_capacity({}, max_staging_size);
      upload.staging.data = staging.map_memory();

      size_t deferred_entity_count = 0;
      this->uploading.pending_upload_counts.for_each([this, &upload, &deferred_entity_count]<typename Entity>(auto& pending_upload_count) {
         if (pending_upload_count <= 0)
            return;
         if (!upload.has_room_for_more({}))
            return;
         //
         bool any_of_this_type = false;
         //
         auto& list = this->scene.entities_of_type<Entity>();
         for (size_t i = 0; i < list.size(); ++i) {
            if (pending_upload_count <= 0)
               break;
            auto& entity = list[i];
            if (entity.lifetime.life_state != scene_entities::life_state::active_pending_upload)
               continue;
            if (!upload.has_room_for_more({})) {
               ++deferred_entity_count;
               continue;
            }

            any_of_this_type = true;
            --pending_upload_count;
            
            upload.align_to({}, entity.owned_gpu_resource_upload_alignment());
            upload.set_entity_index({}, i);
            entity.upload_owned_gpu_resources(upload);
            upload.next({});

            if (entity.lifetime.life_state == scene_entities::life_state::pending_delete) // entities may mark themselves for delete on failure
               continue;
            entity.lifetime.life_state = scene_entities::life_state::active;
            entity.lifetime.sync_state.set_all_out_of_date();
         }

         if (any_of_this_type) {
            for (auto& fif : this->swap_chain.frames_in_flight)
               fif.on_scene_entity_owned_gpu_resource_upload_complete<Entity>();
         }
      });
      if (deferred_entity_count) {
         qDebug("[surface_renderer::_execute_pending_scene_entity_gpu_uploads] Staging buffer soft cap reached; %d entity CPU-to-GPU uploads pushed to next frame.", deferred_entity_count);
      }

      upload.finish_queueing();

      {
         auto submit_info = VkSubmitInfo{
            .sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO,
            .commandBufferCount = 1,
            .pCommandBuffers    = &commands.handle,
         };
         vkResetFences(this->logical_device, 1, &this->uploading.fence);
         if (auto result = vkQueueSubmit(this->queues.graphics.handle, 1, &submit_info, this->uploading.fence); result != VK_SUCCESS) {
            throw result_exception(result, "[surface_renderer::_execute_pending_scene_entity_gpu_uploads] Submission failed.");
         }
         if (auto result = vkWaitForFences(this->logical_device, 1, &this->uploading.fence, VK_FALSE, UINT64_MAX); result != VK_SUCCESS) {
            throw result_exception(result, "[surface_renderer::_execute_pending_scene_entity_gpu_uploads] Wait-for-completion failed.");
         }
      }
      return true;
   }
   void surface_renderer::_wait_on_pending_scene_entity_gpu_uploads() {
      if (auto result = vkWaitForFences(this->logical_device, 1, &this->uploading.fence, VK_FALSE, UINT64_MAX); result != VK_SUCCESS) {
         throw result_exception(result, "[surface_renderer::_execute_pending_scene_entity_gpu_uploads] Wait-for-completion failed.");
      }
      this->uploading.staging = {}; // free the staging buffer after the GPU-to-GPU copy is complete
   }
   void surface_renderer::_execute_pending_scene_entity_deletions() {
      this->scene.entities.pending_deletion_counts.for_each([this]<typename Entity>(size_t& pending_deletion_count) {
         if (pending_deletion_count <= 0)
            return;
         size_t deleted    = 0;
         size_t last_alive = -1;
         auto&  list       = this->scene.entities_of_type<Entity>();
         bool   recycling  = false;
         for (size_t i = 0; i < list.size(); ++i) {
            auto& item = list[i];
            if (!item.lifetime.sync_state.are_all_up_to_date()) {
               last_alive = i;
               continue;
            }
            if (item.pending_delete()) {
               item.reset();
               ++deleted;
            } else {
               if (item.active() && item.recycle_in_progress()) {
                  if constexpr (debug_log_scene_object_lifetimes) {
                     qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_entity_deletions] Carrying out recycle for scene %s #%u...", Entity::name_single, i);
                     if constexpr (scene_entities::concepts::owns_gpu_resources<Entity>) {
                        qDebug(" - Note: Has current GPU resources? %u. Has outdated GPU resources? %u.", item.owned_gpu_resources.has_current(), item.owned_gpu_resources.has_outdated());
                     }
                  }
                  if constexpr (scene_entities::concepts::owns_gpu_resources<Entity>) {
                     item.owned_gpu_resources.destroy_outdated();
                  }
                  item.lifetime.recycling = false;
                  item.lifetime.sync_state.set_all_out_of_date();
                  ++deleted;
                  if constexpr (debug_log_scene_object_lifetimes) {
                     if (item.recycle_in_progress()) {
                        qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_entity_deletions] Scene %s #%u recycled. The entity is now flagged as \"active.\"", Entity::name_single, i);
                     }
                  }
                  recycling = true;
               }
               last_alive = i;
            }
         }
         if constexpr (debug_log_scene_object_lifetimes) {
            if (deleted) {
               qDebug("[vulkanDK::surface_renderer::_execute_pending_scene_entity_deletions] Deleted or recycled %u scene %s.", deleted, Entity::name_plural);
            }
         }
         pending_deletion_count -= deleted;
         list.resize(last_alive + 1);
         //
         if (recycling) {
            for (auto& fif : this->swap_chain.frames_in_flight) {
               fif.on_scene_entity_added_or_removed<Entity>(); // TODO: make a separate function for recycles?
            }
         }
      });
   }
   #pragma endregion
}