#include "lua_managed_resources.h"
#include <cassert>
#include "../../../helpers/qt/repaint.h"

#include <QImage>
#include "../../../../DirectXTex/DirectXTex.h"
#include "../../../helpers/intrusive_windows_defines.h"

namespace {
   static constexpr int resource_resynchronize_interval = 17; // 1000 / 60 == 16.6ms

   // This should be a DXGI format suitable for reading by script. In practice, this needs to be 
   // whatever DXGI format matches whatever QImage format our script code expects. There isn't 
   // really any other constraint on what this can be.
   static constexpr DXGI_FORMAT desired_dds_pixel_format = DXGI_FORMAT_R8G8B8A8_UNORM;
}

namespace editor_script {
   #pragma region LuaManagedResource
   /*static*/ LuaManagedResource* LuaManagedResource::make_dds(const void* buffer, size_t size) {
      assert(buffer && size);
      //
      using namespace DirectX;
      using image_ptr_t = std::unique_ptr<ScratchImage>;
      //
      TexMetadata metadata;
      image_ptr_t raw(new (std::nothrow) ScratchImage);
      HRESULT     hr = LoadFromDDSMemory(buffer, size, DDS_FLAGS_NONE, &metadata, *raw);
      if (FAILED(hr))
         return nullptr;
      //
      if (IsTypeless(metadata.format)) {
         metadata.format = MakeTypelessUNORM(metadata.format);
         if (IsTypeless(metadata.format))
            return nullptr;
         raw->OverrideFormat(metadata.format);
      }
      if (IsPlanar(metadata.format)) {
         //
         // Some DDS files split the image into multiple "planes:" instead of having the R, G, B, and A 
         // values interleaved together, the file effectively stores four single-channel images. We want 
         // to merge those into RGBA.
         //
         image_ptr_t merged(new (std::nothrow) ScratchImage);
         if (!merged)
            return nullptr; // out of memory
         hr = ConvertToSinglePlane(raw->GetImages(), raw->GetImageCount(), metadata, *merged);
         if (FAILED(hr))
            return nullptr;
         metadata = merged->GetMetadata();
         raw.swap(merged);
      }
      //
      if (IsCompressed(metadata.format)) {
         image_ptr_t decompressed(new (std::nothrow) ScratchImage);
         if (!decompressed)
            return nullptr; // out of memory
         Decompress(raw->GetImages(), raw->GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, *decompressed);
         std::swap(decompressed, raw);
         metadata = raw->GetMetadata();
      }
      if (metadata.format != desired_dds_pixel_format) {
         image_ptr_t converted(new (std::nothrow) ScratchImage);
         if (!converted)
            return nullptr; // out of memory
         hr = Convert(raw->GetImages(), raw->GetImageCount(), metadata, desired_dds_pixel_format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *converted);
         if (FAILED(hr))
            return nullptr;
         std::swap(converted, raw);
         metadata = raw->GetMetadata();
      }
      //
      if (HasAlpha(metadata.format) && metadata.IsPMAlpha()) {
         //
         // If alpha needs to be premultiplied, handle it. Note that PremultiplyAlpha returns an 
         // error code on images that don't need PMA, so we actually do have to check that.
         //
         image_ptr_t mod(new (std::nothrow) ScratchImage);
         if (!mod)
            return nullptr; // out-of-memory
         hr = PremultiplyAlpha(raw->GetImages(), raw->GetImageCount(), metadata, TEX_PMALPHA_REVERSE, *mod);
         if (FAILED(hr))
            return nullptr;
         metadata = mod->GetMetadata();
         raw.swap(mod);
      }
      //
      auto* out = new LuaManagedResource;
      out->type = lua_managed_resource_type::dds;
      out->content.dds.data = raw.release();
      out->content.dds.info = new TexMetadata(metadata);
      return out;
   }

   void LuaManagedResource::modify_raster_script_side(std::function<void(QImage&)> task) {
      DovahKitScriptVMResourceInterface::get().modify_raster_script_side(*this, task);
   }

   bool LuaManagedResource::is_cubemap() const noexcept {
      if (!this->is_dds())
         return false;
      auto* info = this->content.dds.info;
      if (!info)
         return false;
      return info->IsCubemap();
   }
   size_t LuaManagedResource::texture_array_size() const noexcept { // NOTE: textures in an array can be mipmapped
      if (!this->is_dds())
         return 0;
      auto* info = this->content.dds.info;
      if (!info)
         return 0;
      if (info->IsCubemap())
         return (info->arraySize / 6);
      return info->arraySize;
   }
   size_t LuaManagedResource::mipmap_count() const noexcept {
      if (!this->is_dds())
         return 0;
      auto* info = this->content.dds.info;
      if (!info)
         return 0;
      return info->mipLevels - 1; // return 0 for non-mipmapped images
   }
   QImage LuaManagedResource::get_dds_layer(size_t array_index, size_t mipmap_index, uint8_t cubemap_face) const noexcept {
      if (!this->is_dds())
         return QImage();
      auto* data = this->content.dds.data;
      auto* info = this->content.dds.info;
      if (!data || !info)
         return QImage();
      if (array_index >= info->arraySize)
         return QImage();
      if (mipmap_index >= info->mipLevels)
         return QImage();
      //
      size_t ai = array_index;
      if (info->IsCubemap()) {
         if (cubemap_face >= 6)
            return QImage();
         ai = (array_index * 6) + cubemap_face;
         if (ai >= info->arraySize)
            return QImage();
      } else {
         if (cubemap_face > 0)
            return QImage();
         cubemap_face = 0;
      }
      auto* layer = data->GetImage(mipmap_index, ai, 0);
      if (!layer)
         return QImage();
      if (layer->width > std::numeric_limits<int>::max())
         return QImage();
      if (layer->height > std::numeric_limits<int>::max())
         return QImage();
      if (layer->rowPitch > std::numeric_limits<int>::max())
         return QImage();
      auto qt_image = QImage((const uchar*)layer->pixels, layer->width, layer->height, layer->rowPitch, QImage::Format_ARGB32);
      assert(!qt_image.isNull());
      qt_image.detach();
      assert(qt_image.isDetached());
      assert(qt_image.constBits() != layer->pixels);
      return qt_image;
   }

   void LuaManagedResource::on_referenced() {
      if (++this->refcount == 1)
         DovahKitScriptVMResourceInterface::get().on_resource_ui_referenced_changed(*this, true);
   }
   void LuaManagedResource::on_severed() {
      --this->refcount;
      //
      assert(this->refcount >= 0);
      if (this->refcount == 0) {
         DovahKitScriptVMResourceInterface::get().on_resource_ui_referenced_changed(*this, false);
      }
   }

   void LuaManagedResource::resynchronize() {
      switch (this->type) {
         case decltype(this->type)::dds:
            {
               auto data = this->get_dds_layer(0, 0);
               if (!data.isNull())
                  this->content.raster.client = QPixmap::fromImage(this->get_dds_layer(0, 0));
            }
            break;
         case decltype(this->type)::raster:
            {
               auto& raster = this->content.raster;
               if (raster.script.isNull()) {
                  if (!raster.client.isNull()) {
                     raster.client = QPixmap();
                     break;
                  }
                  return;
               }
               raster.client = QPixmap::fromImage(raster.script);
            }
            break;
         default:
            return;
      }
      emit resynchronized();
   }
   #pragma endregion
}

void DovahKitScriptItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
   QStyledItemDelegate::initStyleOption(option, index);
   //
   auto data = index.data(Qt::DecorationRole);
   if (auto* resource = editor_script::LuaManagedResourceHandle::extract_from_variant(data)) {
      switch (resource->resource_type()) {
         using lmrt = editor_script::lua_managed_resource_type;
         case lmrt::dds:
         case lmrt::raster:
            break;
         default:
            return;
      }
      const auto pm = resource->get_raster_widget_side();
      if (pm.isNull())
         return;
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

void DovahKitScriptVMResourceInterface::main_thread_handler() {
   DovahKitScriptVMCore::require_client_thread();
   //
   auto& vm = DovahKitScriptVMCore::get();
   if (!this->timer.isValid() || this->timer.elapsed() > resource_resynchronize_interval) {
      this->timer.start();
      //
      // Synchronize any resource edits made on the script thread, over to the main thread. 
      // Force any widgets using these resources within model items to repaint. (Widgets 
      // using these resources in and of themselves can hook the "resynchronized" signal.)
      // 
      auto& base = this->resources.desynched;
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
      if (update) {
         for (auto* window : vm.widgets.windows)
            cobb::qt::update_hierarchy(window);
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
   resource->type = editor_script::lua_managed_resource_type::raster;
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
DovahKitScriptVMResourceInterface::resource_t* DovahKitScriptVMResourceInterface::create_resource(const QByteArray& source, editor_script::lua_managed_resource_type type) {
   DovahKitScriptVMCore::require_client_thread();
   //
   using lmrt = editor_script::lua_managed_resource_type;
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
   if (type == lmrt::dds) { // TODO: only add to the list when it actually becomes used in the UI
      auto& base = this->resources.desynched;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      //
      list.push_back(resource);
   }
   return resource;
}

void DovahKitScriptVMResourceInterface::modify_raster_script_side(resource_t& r, std::function<void(QImage&)> task) {
   if (r.type != editor_script::lua_managed_resource_type::raster)
      return;
   auto& base = this->resources.desynched;
   auto& list = base.list;
   std::unique_lock guard(base.lock);
   //
   (task)(r.content.raster.script);
   list.push_back(&r);
}

void DovahKitScriptVMResourceInterface::on_resource_ui_referenced_changed(resource_t& resource, bool became_referenced) {
   if (became_referenced) {
      // ...
   } else {
      this->on_resource_unreferenced(resource);
   }
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
      auto& base = this->resources.pending_deletion;
      auto& list = base.list;
      std::unique_lock guard(base.lock);
      assert(!list.contains(&resource));
      list.push_back(&resource);
   }
}
#pragma endregion