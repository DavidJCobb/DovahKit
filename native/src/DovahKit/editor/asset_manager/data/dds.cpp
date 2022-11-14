#include "dds.h"
#include "../../../../DirectXTex/DirectXTex.h"
#include "helpers/windows.h"
#include "dovah/files/bsa/bsa_archived_file.h"

namespace {
   static constexpr DXGI_FORMAT    desired_dds_pixel_format = DXGI_FORMAT_B8G8R8A8_UNORM;
   static constexpr QImage::Format desired_dds_pixel_qt_fmt = QImage::Format_ARGB32; // the Qt equivalent of desired_dds_pixel_format

   QImage _qt_image_from_dds(DirectX::ScratchImage* image) {
      auto* layer = image->GetImage(0, 0, 0);
      if (!layer)
         return QImage();
      if (layer->width > std::numeric_limits<int>::max())
         return QImage();
      if (layer->height > std::numeric_limits<int>::max())
         return QImage();
      if (layer->rowPitch > std::numeric_limits<int>::max())
         return QImage();
      return QImage((const uchar*)layer->pixels, layer->width, layer->height, layer->rowPitch, desired_dds_pixel_qt_fmt);
   }
}

bool DovahKitAssetDataDDS::load(const dovah::bsa_archived_file& file) {
   using namespace DirectX;
   using image_ptr_t = std::unique_ptr<ScratchImage>;
   //
   TexMetadata metadata;
   image_ptr_t raw(new (std::nothrow) ScratchImage);
   HRESULT     hr = LoadFromDDSMemory(file.data(), file.size(), DDS_FLAGS_NONE, &metadata, *raw);
   if (FAILED(hr))
      return false;
   //
   if (IsTypeless(metadata.format)) {
      metadata.format = MakeTypelessUNORM(metadata.format);
      if (IsTypeless(metadata.format))
         return false;
      raw->OverrideFormat(metadata.format);
   }
   if (IsPlanar(metadata.format)) {
      //
      // Some DDS files split the image into multiple "planes:" instead of having the R, G, B, and A 
      // values interleaved together, the file effectively stores four single-channel images. We want 
      // to merge those into RGBA.
      //
      image_ptr_t merged(new (std::nothrow) ScratchImage);
      if (!merged) // out of memory
         return false;
      hr = ConvertToSinglePlane(raw->GetImages(), raw->GetImageCount(), metadata, *merged);
      if (FAILED(hr))
         return false;
      metadata = merged->GetMetadata();
      raw.swap(merged);
   }
   //
   if (IsCompressed(metadata.format)) {
      image_ptr_t decompressed(new (std::nothrow) ScratchImage);
      if (!decompressed) // out of memory
         return false;
      Decompress(raw->GetImages(), raw->GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, *decompressed);
      std::swap(decompressed, raw);
      metadata = raw->GetMetadata();
   }
   if (metadata.format != desired_dds_pixel_format) {
      image_ptr_t converted(new (std::nothrow) ScratchImage);
      if (!converted) // out of memory
         return false;
      hr = Convert(raw->GetImages(), raw->GetImageCount(), metadata, desired_dds_pixel_format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *converted);
      if (FAILED(hr))
         return false;
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
      if (!mod) // out of memory
         return false;
      hr = PremultiplyAlpha(raw->GetImages(), raw->GetImageCount(), metadata, TEX_PMALPHA_REVERSE, *mod);
      if (FAILED(hr))
         return false;
      metadata = mod->GetMetadata();
      raw.swap(mod);
   }
   //
   this->data  = raw.release();
   this->info  = new TexMetadata(metadata);
   this->image = _qt_image_from_dds(this->data);
   return true;
}