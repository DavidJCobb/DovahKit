#include "lua_managed_resources.h"

namespace editor_script {
   #pragma region LuaManagedResource
   void LuaManagedResource::mark_dirty() {
      DovahKitScriptVMResourceInterface::get().mark_dirty(*this);
   }

   void LuaManagedResource::on_referenced(model_t* m) {
      ++this->refcount;
      //
      if (m)
         this->model_refcounts[m] += 1;
   }
   void LuaManagedResource::on_severed(model_t* m) {
      --this->refcount;
      //
      if (m) {
         auto& map = this->model_refcounts;
         auto  it  = map.find(m);
         assert(it != map.end());
         //
         int v = (it.value() -= 1);
         assert(v >= 0);
         if (v == 0)
            map.erase(it);
      }
      //
      assert(this->refcount >= 0);
      if (this->refcount == 0)
         DovahKitScriptVMResourceInterface::get().on_resource_unreferenced(*this);
   }

   void LuaManagedResource::resynchronize() {
      auto& raster = this->content.raster;
      if (raster.script.isNull()) {
         if (!raster.client.isNull()) {
            raster.client = QPixmap();
            emit resynchronized();
         }
         return;
      }
      raster.client = QPixmap::fromImage(raster.script);
      emit resynchronized();
   }
   #pragma endregion
}

#pragma region DovahKitScriptVMResourceInterface
void DovahKitScriptVMResourceInterface::clear() {
   DovahKitScriptVMCore::require_client_thread();
   //
   {
      auto& base = this->resources.desynched;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      list.clear();
   }
   {
      auto& base = this->resources.extant;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      for (auto* resource : list) {
         assert(resource);
         delete resource;
      }
      list.clear();
   }
   {
      auto& base = this->resources.pending_deletion;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      for (auto* resource : list) {
         assert(resource);
         delete resource;
      }
      list.clear();
   }
}

void DovahKitScriptVMResourceInterface::main_thread_handler() {
   DovahKitScriptVMCore::require_client_thread();
   //
   auto& vm = DovahKitScriptVMCore::get();
   {
      //
      // Synchronize any resource edits made on the script thread, over to the main thread. 
      // Force any widgets using these resources within model items to repaint. (Widgets 
      // using these resources in and of themselves can hook the "resynchronized" signal.)
      // 
      // TODO: Throttle this part of the main thread handler to 60 FPS.
      //
      auto& base = this->resources.desynched;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      QList<QWidget*> widgets_to_update;
      for (auto* resource : list) {
         assert(resource);
         resource->resynchronize();
         //
         for (auto* model : resource->model_refcounts.keys()) {
            auto widgets = vm._get_scripted_widgets_using_model(model);
            for (auto* w : widgets)
               if (!widgets_to_update.contains(w))
                  widgets_to_update.push_back(w);
         }
      }
      list.clear();
      //
      for (auto* w : widgets_to_update)
         w->update();
   }
   {
      auto& base = this->resources.pending_deletion;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      for (auto* resource : list) {
         assert(resource);
         delete resource;
      }
      list.clear();
   }
}

DovahKitScriptVMResourceInterface::resource_t* DovahKitScriptVMResourceInterface::create_resource(QImage source) {
   DovahKitScriptVMCore::require_client_thread();
   //
   resource_t* resource = new resource_t;
   {
      auto& base = this->resources.extant;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      list.push_back(resource);
   }
   if (!source.isNull()) {
      resource->content.raster.script = source;
      //
      auto& base = this->resources.desynched;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      list.push_back(resource);
   }
   return resource;
}

void DovahKitScriptVMResourceInterface::on_resource_unreferenced(resource_t& resource) {
   if (resource.is_lua_referenced)
      return;
   if (resource.refcount)
      return;
   {
      std::unique_lock guard(this->resources.desynched.lock);
      this->resources.desynched.list.removeAll(&resource);
   }
   {
      std::unique_lock guard(this->resources.extant.lock);
      this->resources.extant.list.removeAll(&resource);
   }
   {
      std::unique_lock guard(this->resources.pending_deletion.lock);
      this->resources.pending_deletion.list.push_back(&resource);
   }
   delete &resource;
}

void DovahKitScriptVMResourceInterface::mark_dirty(resource_t& resource) {
   std::unique_lock guard(this->resources.desynched.lock);
   auto& list = this->resources.desynched.list;
   if (list.contains(&resource))
      return;
   list.push_back(&resource);
}
#pragma endregion