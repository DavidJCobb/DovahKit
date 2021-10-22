#include "asset.h"
#include "../../../DirectXTex/DirectXTex.h"
#include "../../helpers/intrusive_windows_defines.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include "../core.h"

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

#pragma region DovahKitAsset
DovahKitAsset::DovahKitAsset(Type t, QObject* parent) : QObject(parent) {
   this->_type = t;
}
DovahKitAsset::~DovahKitAsset() {
   if (auto*& p = this->_data.dds.data) {
      delete p;
      p = nullptr;
   }
   if (auto*& p = this->_data.dds.info) {
      delete p;
      p = nullptr;
   }
}

void DovahKitAsset::load(const QString& path) {
   using file_type = dovah::bsa_archived_file;
   this->_state.content_failed      = false;
   this->_state.content_loaded      = false;
   this->_state.dependencies_loaded = false;
   //
   if (this->_type == Type::Undefined) {
      this->_state.content_failed = true;
      emit contentLoadingFailed();
      return;
   }
   file_type* file = nullptr;
   {
      std::filesystem::path std_path = path.toStdWString();
      file = DovahKitCore::get().lookup_game_asset(std_path, true);
   }
   if (!file) {
      this->_state.content_failed = true;
      emit contentLoadingFailed();
      return;
   }
   //
   // Type-specific loading code:
   //
   if (this->_type == Type::DDS) {
      using namespace DirectX;
      using image_ptr_t = std::unique_ptr<ScratchImage>;
      //
      TexMetadata metadata;
      image_ptr_t raw(new (std::nothrow) ScratchImage);
      HRESULT     hr = LoadFromDDSMemory(file->data(), file->size(), DDS_FLAGS_NONE, &metadata, *raw);
      if (FAILED(hr)) {
         this->_state.content_failed = true;
         emit contentLoadingFailed();
         return;
      }
      //
      if (IsTypeless(metadata.format)) {
         metadata.format = MakeTypelessUNORM(metadata.format);
         if (IsTypeless(metadata.format)) {
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         raw->OverrideFormat(metadata.format);
      }
      if (IsPlanar(metadata.format)) {
         //
         // Some DDS files split the image into multiple "planes:" instead of having the R, G, B, and A 
         // values interleaved together, the file effectively stores four single-channel images. We want 
         // to merge those into RGBA.
         //
         image_ptr_t merged(new (std::nothrow) ScratchImage);
         if (!merged) { // out of memory
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         hr = ConvertToSinglePlane(raw->GetImages(), raw->GetImageCount(), metadata, *merged);
         if (FAILED(hr)) {
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         metadata = merged->GetMetadata();
         raw.swap(merged);
      }
      //
      if (IsCompressed(metadata.format)) {
         image_ptr_t decompressed(new (std::nothrow) ScratchImage);
         if (!decompressed) { // out of memory
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         Decompress(raw->GetImages(), raw->GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, *decompressed);
         std::swap(decompressed, raw);
         metadata = raw->GetMetadata();
      }
      if (metadata.format != desired_dds_pixel_format) {
         image_ptr_t converted(new (std::nothrow) ScratchImage);
         if (!converted) { // out of memory
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         hr = Convert(raw->GetImages(), raw->GetImageCount(), metadata, desired_dds_pixel_format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *converted);
         if (FAILED(hr)) {
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
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
         if (!mod) { // out of memory
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         hr = PremultiplyAlpha(raw->GetImages(), raw->GetImageCount(), metadata, TEX_PMALPHA_REVERSE, *mod);
         if (FAILED(hr)) {
            this->_state.content_failed = true;
            emit contentLoadingFailed();
            return;
         }
         metadata = mod->GetMetadata();
         raw.swap(mod);
      }
      //
      this->_data.dds.data = raw.release();
      this->_data.dds.info = new TexMetadata(metadata);
      this->_data.image = _qt_image_from_dds(this->_data.dds.data);
      //
      this->_state.content_loaded      = true;
      this->_state.dependencies_loaded = true; // DDS files have no external dependencies
      emit this->contentLoaded();
      emit this->ready();
      return;
   }
   //
   // Unrecognized type value:
   //
   this->_state.content_failed = true;
   emit contentLoadingFailed();
   return;
}
#pragma endregion

#pragma region DovahKitAssetHandle
#pragma endregion