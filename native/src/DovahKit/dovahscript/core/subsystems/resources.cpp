#include "resources.h"
#include <QThread>
#include "../../../helpers/qt/repaint.h"
#include "coordinator.h"
#include "../verify_threading.h"
#include "../../constants/debugging.h"

namespace {
   static constexpr int resource_resynchronize_interval = 17; // 1000 / 60 == 16.6ms

   static constexpr bool debug_log_resource_management = dovahscript::force_enable_debug_logging || false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;
}

namespace dovahscript::core::subsystems {
   void resources::on_script_setup() {
      auto guard = std::lock_guard(this->stored_resources.extant.lock);
      assert(this->stored_resources.extant.list.empty());
      assert(this->stored_resources.pending_deletion.list.empty());
      assert(this->stored_resources.desynched.list.empty());
   }
   void resources::main_thread_handler() {
      require_client_thread();
      //
      if (!this->timer.isValid() || this->timer.elapsed() > resource_resynchronize_interval) {
         this->timer.start();
         //
         // Synchronize any resource edits made on the script thread, over to the main thread. 
         // Force any widgets using these resources within model items to repaint. (Widgets 
         // using these resources in and of themselves can hook the "resynchronized" signal.)
         // 
         auto& base = this->stored_resources.desynched;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         bool update = false;
         for (auto* resource : list) {
            assert(resource);
            resource->resynchronize();
            if (!update && resource->refcount) // check (update) first to avoid delay on checking the atomic refcount
               update = true;
         }
         list.clear();
         if (update)
            coordinator::get().force_ui_repaint();
      }
      {
         auto& base = this->stored_resources.pending_deletion;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         for (auto* resource : list) {
            assert(resource);
            if constexpr (debug_log_resource_management)
               qDebug("Deleting Lua-managed resource: %p", resource);
            resource->deleteLater();
         }
         list.clear();
      }
   }
   void resources::on_script_teardown() {
      require_client_thread();
      //
      {
         auto& base = this->stored_resources.desynched;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         list.clear();
      }
      //
      // Resource deletions done during teardown need to be direct instead of 
      // relying on QObject::deleteLater as normal. This is due to the order of 
      // operations during teardown:
      // 
      //  - Handle resources, and then forget about them
      //  - Handle widgets, and then forget about them
      // 
      // Widgets are handled by running deleteLater, which means that if a widget 
      // has a DovahscriptResourceHandle, then it will clear that handle on the 
      // next program tick *after* teardown is complete. This will in turn cause 
      // us to attempt to handle the case of the resource becoming unreferenced 
      // after we've already forgotten that the resource exists.
      // 
      // If we simply delete the resource here, then the DovahscriptResourceHandles 
      // will receive its "destroyed" signal and sever their references to it, which 
      // avoids the problem.
      //
      {
         auto& base = this->stored_resources.extant;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         for (auto* resource : list) {
            assert(resource);
            assert(QThread::currentThread() == resource->thread());
            if constexpr (debug_log_resource_management)
               qDebug("Teardown is deleting extant Lua-managed resource: %p", resource);
            delete resource;
         }
         list.clear();
      }
      {
         auto& base = this->stored_resources.pending_deletion;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         for (auto* resource : list) {
            assert(resource);
            assert(QThread::currentThread() == resource->thread());
            if constexpr (debug_log_resource_management)
               qDebug("Teardown is deleting marked-for-delete Lua-managed resource: %p", resource);
            delete resource;
         }
         list.clear();
      }
   }

   resources::resource_t* resources::create_resource(QImage source) {
      require_client_thread();
      //
      resource_t* resource = new resource_t;
      assert(resource);
      resource->type = resource_type::raster;
      if constexpr (debug_log_resource_management)
         qDebug("Creating Lua-managed resource: %p", resource);
      {
         auto& base = this->stored_resources.extant;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         list.push_back(resource);
      }
      if (!source.isNull()) {
         resource->content.raster.script = source;
         //
         auto& base = this->stored_resources.desynched;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         list.push_back(resource);
      }
      return resource;
   }
   resources::resource_t* resources::create_resource(const QByteArray& source, resource_type type) {
      require_client_thread();
      //
      using lmrt = resource_type;
      //
      resource_t* resource = nullptr;
      if (type == lmrt::binary) {
         resource = new resource_t;
         resource->type = type;
         resource->content.binary = source;
         resource->content.binary.detach();
      } else if (type == lmrt::dds) {
         resource = resource_t::make_dds(source.constData(), source.size());
         if (!resource)
            return nullptr;
      } else if (type == lmrt::raster) {
         assert(false && "Cannot create a raster resource from a buffer alone; we don't know the image size.");
      }
      assert(resource);
      if constexpr (debug_log_resource_management)
         qDebug("Creating Lua-managed resource: %p", resource);
      {
         auto& base = this->stored_resources.extant;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         list.push_back(resource);
      }
      if (type == lmrt::dds) { // TODO: only add to the list when it actually becomes used in the UI
         auto& base = this->stored_resources.desynched;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         //
         list.push_back(resource);
      }
      return resource;
   }

   void resources::modify_raster_script_side(resource_t& r, std::function<void(QImage&)> task) {
      require_script_thread();
      //
      if (r.type != resource_type::raster)
         return;
      auto& base = this->stored_resources.desynched;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      (task)(r.content.raster.script);
      if (!list.contains(&r))
         list.push_back(&r);
   }

   void resources::on_resource_ui_referenced_changed(resource_t& resource, bool became_referenced) {
      if (became_referenced) {
         // ...
      } else {
         this->on_resource_unreferenced(resource);
      }
   }
   void resources::on_resource_unreferenced(resource_t& resource) {
      if constexpr (debug_log_resource_management)
         qDebug("Lua-managed resource has become unreferenced either within Lua or Qt: %p", &resource);
      if (resource.is_lua_referenced)
         return;
      if (resource.refcount)
         return;
      if constexpr (debug_log_resource_management)
         qDebug("Marking Lua-managed resource for delete: %p", &resource);
      bool present;
      {
         std::unique_lock guard_a(this->stored_resources.desynched.lock);
         std::unique_lock guard_b(this->stored_resources.extant.lock);
         this->stored_resources.desynched.list.removeAll(&resource);
         int i = this->stored_resources.extant.list.removeAll(&resource);
         present = i > 0;
      }
      if (!present) {
         //
         // The resource isn't present in the extant-list. This can happen during script teardown: 
         // we delete the resources from our teardown handler, and after that, a userdata referring 
         // to the resource is deleted, bringing us here.
         // 
         assert(coordinator::get().teardown_in_progress());
         //
         // Proper behavior in this case is to just return early; we DO NOT want to add the resource 
         // to the pending deletion list. It's already had deleteLater called on it, so if we add it 
         // to the pending deletion list, then immediately upon starting the *next* script session, 
         // we'll delete the already-deleted resource.
         //
         if constexpr (debug_log_resource_management)
            qDebug(" - Resource not present in the extant-list. This can happen if the VM has already torn down.");
         return;
      }
      {
         auto& base = this->stored_resources.pending_deletion;
         auto& list = base.list;
         std::unique_lock guard(base.lock);
         assert(!list.contains(&resource));
         list.push_back(&resource);
      }
   }
}