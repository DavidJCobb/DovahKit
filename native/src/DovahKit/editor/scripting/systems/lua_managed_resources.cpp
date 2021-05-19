#include "lua_managed_resources.h"

// Needed to force updates to a combobox's body
#include <QAbstractItemView>
#include <QComboBox>

// For debugging
#if _DEBUG
   #include "userdata.h"
#endif

namespace editor_script {
   #pragma region LuaManagedResource
   void LuaManagedResource::modify_raster_script_side(std::function<void(QImage)> task) {
      DovahKitScriptVMResourceInterface::get().modify_raster_script_side(*this, task);
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

void DovahKitScriptItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
   QStyledItemDelegate::initStyleOption(option, index);
   //
   auto data = index.data(Qt::DecorationRole);
   if (auto* resource = editor_script::LuaManagedResourceHandle::extract_from_variant(data)) {
      const auto pm = resource->get_raster_widget_side();
      if (!pm.isNull()) {
         option->icon = QIcon(pm);
         {
            //option->decorationSize = pm.size() / pm.devicePixelRatio(); // displays icon at actual size
            auto size = pm.size() / pm.devicePixelRatio();
            auto max  = option->decorationSize;
            if (size.height() > max.height()) {
               option->decorationSize = size.boundedTo(max);
            } else if (size.height() < max.height()) {
               option->decorationSize = size.expandedTo(max);
            }
         }
         option->features |= QStyleOptionViewItem::HasDecoration;
      }
   }
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
         resource->deleteLater();
      }
      list.clear();
   }
}

void DovahKitScriptVMResourceInterface::_run_queued_debug_functions() {
   #if !_DEBUG
      return;
   #endif
   if (this->debug.double_check_lua_references) {
      auto& base = this->resources.extant;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      auto& intfc = DovahKitScriptVMUserdataInterface::get();
      for (auto* resource : list) {
         assert(resource);
         if (!resource->is_lua_referenced)
            continue;
         assert(intfc.wrapper_exists_for(resource));
      }
      this->debug.double_check_lua_references = false;
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
      for (auto* w : widgets_to_update) {
         const auto* mt = w->metaObject();
         if (mt->inherits(&QComboBox::staticMetaObject)) {
            if (auto* vw = ((QComboBox*)w)->view())
               if (auto* vp = vw->viewport())
                  vp->update();
         } else {
            w->update();
         }
      }
   }
   {
      auto& base = this->resources.pending_deletion;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      for (auto* resource : list) {
         #if _DEBUG
            assert(resource);
            qDebug("Deleting Lua-managed resource: %p", resource);
         #endif
         resource->deleteLater();
      }
      list.clear();
   }
}

DovahKitScriptVMResourceInterface::resource_t* DovahKitScriptVMResourceInterface::create_resource(QImage source) {
   DovahKitScriptVMCore::require_client_thread();
   //
   resource_t* resource = new resource_t;
   #if _DEBUG
      qDebug("Creating Lua-managed resource: %p", resource);
   #endif
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

void DovahKitScriptVMResourceInterface::modify_raster_script_side(resource_t& r, std::function<void(QImage&)> task) {
   auto& base = this->resources.desynched;
   auto& list = base.list;
   std::unique_lock guard(base.lock);
   //
   (task)(r.content.raster.script);
   list.push_back(&r);
}

void DovahKitScriptVMResourceInterface::on_resource_unreferenced(resource_t& resource) {
   #if _DEBUG
      qDebug("Lua-managed resource has become unreferenced either within Lua or Qt: %p", &resource);
   #endif
   if (resource.is_lua_referenced)
      return;
   if (resource.refcount)
      return;
   #if _DEBUG
      qDebug("Marking Lua-managed resource for delete: %p", &resource);
   #endif
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
}
#pragma endregion