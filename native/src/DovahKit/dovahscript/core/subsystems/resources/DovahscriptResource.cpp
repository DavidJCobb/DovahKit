#include "DovahscriptResource.h"
#include <QPainter>
#include "../../../../../DirectXTex/DirectXTex.h"
#include "../../../../helpers/intrusive_windows_defines.h"
#include "../../../constants/qt_graphics.h"

#include "../resources.h"
#include "../../verify_threading.h"

namespace {
   // This should be a DXGI format suitable for reading by script. In practice, this needs to be 
   // whatever DXGI format matches whatever QImage format our script code expects. There isn't 
   // really any other constraint on what this can be.
   static constexpr DXGI_FORMAT    desired_dds_pixel_format = DXGI_FORMAT_B8G8R8A8_UNORM;
   static constexpr QImage::Format desired_dds_pixel_qt_fmt = QImage::Format_ARGB32; // the Qt equivalent of desired_dds_pixel_format
}

namespace dovahscript {
   DovahscriptResource::~DovahscriptResource() {
      if (auto*& p = this->content.dds.data) {
         delete p;
         p = nullptr;
      }
      if (auto*& p = this->content.dds.info) {
         delete p;
         p = nullptr;
      }
   }

   /*static*/ DovahscriptResource* DovahscriptResource::make_dds(const void* buffer, size_t size) {
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
      auto* out = new DovahscriptResource;
      out->type = resource_type::dds;
      out->content.dds.data = raw.release();
      out->content.dds.info = new TexMetadata(metadata);
      return out;
   }

   void DovahscriptResource::reserve_binary_script_side(size_t size) {
      core::subsystems::resources::get().reserve_binary_script_side(*this, size);
   }
   void DovahscriptResource::modify_binary_script_side(std::function<void(QByteArray&)> task) {
      core::subsystems::resources::get().modify_binary_script_side(*this, task);
   }
   void DovahscriptResource::modify_raster_script_side(std::function<void(QImage&)> task) {
      core::subsystems::resources::get().modify_raster_script_side(*this, task);
   }

   bool DovahscriptResource::is_cubemap() const noexcept {
      if (!this->is_dds())
         return false;
      auto* info = this->content.dds.info;
      if (!info)
         return false;
      return info->IsCubemap();
   }
   size_t DovahscriptResource::texture_array_size() const noexcept { // NOTE: textures in an array can be mipmapped
      if (!this->is_dds())
         return 0;
      auto* info = this->content.dds.info;
      if (!info)
         return 0;
      if (info->IsCubemap())
         return (info->arraySize / 6);
      return info->arraySize;
   }
   size_t DovahscriptResource::mipmap_count() const noexcept {
      if (!this->is_dds())
         return 0;
      auto* info = this->content.dds.info;
      if (!info)
         return 0;
      return info->mipLevels - 1; // return 0 for non-mipmapped images
   }
   QImage DovahscriptResource::get_dds_layer(size_t array_index, size_t mipmap_index, uint8_t cubemap_face) const noexcept {
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
      QImage qt_image = QImage((const uchar*)layer->pixels, layer->width, layer->height, layer->rowPitch, desired_dds_pixel_qt_fmt);
      if constexpr (desired_dds_pixel_qt_fmt != desired_qt_pixel_format) {
         qt_image.convertTo(desired_qt_pixel_format);
      }
      assert(!qt_image.isNull());
      qt_image.detach();
      assert(qt_image.isDetached());
      assert(qt_image.constBits() != layer->pixels);
      return qt_image;
   }

   void DovahscriptResource::on_referenced(bool ui) {
      if (++this->refcount == 1) {
         core::subsystems::resources::get().on_resource_c_referenced_changed(*this, true);
      }
      if (++this->ui_refcount == 1) {
         core::subsystems::resources::get().on_resource_ui_referenced_changed(*this, true);
      }
   }
   void DovahscriptResource::on_severed(bool ui) {
      --this->refcount;
      //
      assert(this->refcount >= 0);
      if (this->refcount == 0) {
         core::subsystems::resources::get().on_resource_c_referenced_changed(*this, false);
         return;
      }
      if (ui) {
         if (--this->ui_refcount == 0) {
            core::subsystems::resources::get().on_resource_ui_referenced_changed(*this, false);
         }
      }
   }

   void DovahscriptResource::resynchronize() {
      switch (this->type) {
         using _ = decltype(this->type);
         case _::binary:
            {
               auto& binary = this->content.binary;
               auto& src = binary.script;
               auto& dst = binary.client;
               if (src.isNull()) {
                  if (!dst.isNull()) {
                     dst = QByteArray();
                     break;
                  }
                  return;
               }
               if (!dst.isNull() && dst.size() == src.size()) {
                  memcpy(dst.data(), src.constData(), src.size());
               } else {
                  dst = src;
                  dst.detach();
               }
            }
            break;
         case _::dds:
            {
               auto data = this->get_dds_layer(0, 0);
               if (!data.isNull())
                  this->content.raster.client = data;
               //
               // No further logic needed (e.g. to determine whether we emit resynchronized), 
               // as editing DDSes at run-time is not supported.
               //
            }
            break;
         case _::raster:
            {
               auto& raster = this->content.raster;
               auto& src = raster.script;
               auto& dst = raster.client;
               if (src.isNull()) {
                  if (!dst.isNull()) {
                     dst = QImage();
                     break;
                  }
                  return;
               }
               if (!dst.isNull() && src.size() == dst.size() && src.format() == dst.format()) {
                  //
                  // If the image size or format have not changed, then updating it this way should 
                  // hopefully avoid having to free and allocate entirely new data for the client-
                  // thread QImage.
                  //
                  assert(dst.sizeInBytes() == src.sizeInBytes());
                  memcpy(dst.bits(), src.constBits(), src.sizeInBytes());
               } else {
                  //
                  // If the image size or format has changed, replace the old client-thread data with 
                  // a new copy of the worker-thread data.
                  //
                  dst = src;
                  dst.detach();
               }
            }
            break;
         default:
            return;
      }
      emit resynchronized();
   }
   void DovahscriptResource::abandon_client_thread_content() {
      this->content.raster.client = QImage();
   }
}