#include "asset.h"
#include "../../../DirectXTex/DirectXTex.h"
#include "../../helpers/intrusive_windows_defines.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include "../core.h"
#include "asset_manager.h"

namespace {
   // see also: the same constexpr value in asset_manager.cpp
   static constexpr bool debug_asset_lifetime = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;
}

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

   using passkey_to_manager = cobb::passkey<DovahKitAsset, DovahKitAssetManager>;
}

#pragma region DovahKitAsset
DovahKitAsset::DovahKitAsset(QString p, Type t, QObject* parent) : QObject(parent), _type(t), _path(p) {
}
DovahKitAsset::~DovahKitAsset() {
   if constexpr (debug_asset_lifetime) {
      qDebug("Running DovahKitAsset destructor: %p (%s)", this, qUtf8Printable(this->_path));
   }
   this->unload();
}

void DovahKitAsset::on_handle_made(DovahKitAssetReceptor& handle) {
   ++this->_state.refcounts.all;
   if (handle.flags & DovahKitAssetReceptor::Flag::IsRenderWindow)
      ++this->_state.refcounts.render_window;
}
void DovahKitAsset::on_handle_lost(DovahKitAssetReceptor& handle) {
   auto to = --this->_state.refcounts.all;
   if (handle.flags & DovahKitAssetReceptor::Flag::IsRenderWindow)
      --this->_state.refcounts.render_window;
   //
   if (to == 0)
      DovahKitAssetManager::get().onUnreferenced(passkey_to_manager(), *this);
}

void DovahKitAsset::load() {
   auto _fail = [this]() {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset failed to load: %p (%s)", this, qUtf8Printable(this->_path));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.load_requested = false;
      this->_state.content_failed = true;
      emit contentLoadingFailed();
   };
   auto _done = [this]() {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset loaded: %p (%s)", this, qUtf8Printable(this->_path));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.load_requested      = false;
      this->_state.content_loaded      = true;
      this->_state.dependencies_loaded = true; // DDS files have no external dependencies
      emit this->contentLoaded();
      emit this->ready();
   };
   
   using file_type = dovah::bsa_archived_file;
   {
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.content_failed      = false;
      this->_state.content_loaded      = false;
      this->_state.dependencies_loaded = false;
   }
   //
   if (this->_type == Type::Undefined)
      return _fail();
   file_type* file = nullptr;
   {
      std::filesystem::path std_path = this->_path.toStdWString();
      file = DovahKitCore::get().lookup_game_asset(std_path, true);
   }
   if (!file)
      return _fail();
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
      if (FAILED(hr))
         return _fail();
      //
      if (IsTypeless(metadata.format)) {
         metadata.format = MakeTypelessUNORM(metadata.format);
         if (IsTypeless(metadata.format))
            return _fail();
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
            return _fail();
         hr = ConvertToSinglePlane(raw->GetImages(), raw->GetImageCount(), metadata, *merged);
         if (FAILED(hr))
            return _fail();
         metadata = merged->GetMetadata();
         raw.swap(merged);
      }
      //
      if (IsCompressed(metadata.format)) {
         image_ptr_t decompressed(new (std::nothrow) ScratchImage);
         if (!decompressed) // out of memory
            return _fail();
         Decompress(raw->GetImages(), raw->GetImageCount(), metadata, DXGI_FORMAT_UNKNOWN, *decompressed);
         std::swap(decompressed, raw);
         metadata = raw->GetMetadata();
      }
      if (metadata.format != desired_dds_pixel_format) {
         image_ptr_t converted(new (std::nothrow) ScratchImage);
         if (!converted) // out of memory
            return _fail();
         hr = Convert(raw->GetImages(), raw->GetImageCount(), metadata, desired_dds_pixel_format, TEX_FILTER_DEFAULT, TEX_THRESHOLD_DEFAULT, *converted);
         if (FAILED(hr))
            return _fail();
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
            return _fail();
         hr = PremultiplyAlpha(raw->GetImages(), raw->GetImageCount(), metadata, TEX_PMALPHA_REVERSE, *mod);
         if (FAILED(hr))
            return _fail();
         metadata = mod->GetMetadata();
         raw.swap(mod);
      }
      //
      this->_data.dds.data = raw.release();
      this->_data.dds.info = new TexMetadata(metadata);
      this->_data.image    = _qt_image_from_dds(this->_data.dds.data);
      //
      return _done();
   }
   //
   // Unrecognized type value:
   //
   return _fail();
}
void DovahKitAsset::unload() {
   if constexpr (debug_asset_lifetime) {
      qDebug("Unloading DovahKitAsset: %p (%s)", this, qUtf8Printable(this->_path));
   }
   auto guard = std::unique_lock(this->_locks.load_state);
   this->_state.content_failed      = false;
   this->_state.content_loaded      = false;
   this->_state.dependencies_loaded = false;
   //
   if (auto*& p = this->_data.dds.data) {
      delete p;
      p = nullptr;
   }
   if (auto*& p = this->_data.dds.info) {
      delete p;
      p = nullptr;
   }
   this->_data.image = QImage();
}
#pragma endregion

#pragma region DovahKitAssetTransport
DovahKitAssetTransport::~DovahKitAssetTransport() {
   assert(this->value == nullptr && "a DovahKitAssetTransport failed to make it into a receptor");
}

DovahKitAssetTransport::DovahKitAssetTransport(DovahKitAssetTransport&& other) noexcept {
   this->value = other.value;
   other.value = nullptr;
}
DovahKitAssetTransport& DovahKitAssetTransport::operator=(DovahKitAssetTransport&& other) noexcept {
   this->value = other.value;
   other.value = nullptr;
   return *this;
}
#pragma endregion

#pragma region DovahKitAssetReceptor
void DovahKitAssetReceptor::_acquire(value_type* value) {
   enum class which {
      none,
      ready,
      failed,
   };
   //
   which fire = which::none;
   this->state &= ~state_flag::ready;
   if (value) {
      {
         auto guard = std::unique_lock(value->_locks.load_state);
         this->asset = value;
         if (value->isReady()) {
            this->state |= state_flag::ready;
            fire = which::ready;
         } else if (value->didContentLoadingFail()) {
            this->state |= state_flag::failed;
            fire = which::failed;
         } else {
            if constexpr (debug_asset_lifetime) {
               qDebug("DovahKitAssetReceptor is listening for \"ready\" and \"failed\" signals: %p (%s)", this, qUtf8Printable(this->asset->_path));
            }
            QObject::connect(asset, &value_type::ready, this, &DovahKitAssetReceptor::_forwardReady, Qt::QueuedConnection);
            QObject::connect(asset, &value_type::contentLoadingFailed, this, &DovahKitAssetReceptor::_forwardFailed, Qt::QueuedConnection);
         }
         QObject::connect(asset, &QObject::destroyed, this, &DovahKitAssetReceptor::_forwardUnloaded, Qt::DirectConnection);
      }
      this->asset->on_handle_made(*this);
   }
   switch (fire) {
      case which::ready:
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAssetReceptor is forwarding \"ready\" signal (on acquire): %p (%s)", this, qUtf8Printable(this->asset->_path));
         }
         emit this->ready();
         break;
      case which::failed:
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAssetReceptor is forwarding \"failed\" signal (on acquire): %p (%s)", this, qUtf8Printable(this->asset->_path));
         }
         emit this->failed();
         break;
   }
}
void DovahKitAssetReceptor::_clear() {
   if (auto* p = this->asset.data()) {
      QObject::disconnect(p, nullptr, this, nullptr);
      p->on_handle_lost(*this);
      this->asset = nullptr;
   }
   this->state &= ~(state_flag::ready | state_flag::failed);
}

void DovahKitAssetReceptor::_severLoadSignals() {
   QObject::disconnect(this->asset, &value_type::ready,                this, nullptr);
   QObject::disconnect(this->asset, &value_type::contentLoadingFailed, this, nullptr);
}

void DovahKitAssetReceptor::_forwardFailed() {
   if (this->asset) {
      this->_severLoadSignals();
      this->state |= state_flag::failed;
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAssetReceptor is forwarding \"failed\" signal (after listening): %p (%s)", this, qUtf8Printable(this->asset->_path));
      }
   }
   emit this->failed();
}
void DovahKitAssetReceptor::_forwardReady() {
   if (this->asset) {
      this->_severLoadSignals();
      this->state |= state_flag::ready;
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAssetReceptor is forwarding \"ready\" signal (after listening): %p (%s)", this, qUtf8Printable(this->asset->_path));
      }
   }
   emit this->ready();
}
void DovahKitAssetReceptor::_forwardUnloaded(QObject* target) {
   emit this->unloaded();
}

bool DovahKitAssetReceptor::isReady() const noexcept {
   if (this->asset) {
      /*//
      auto guard = std::unique_lock(this->asset->_locks.load_state);
      return this->asset->isReady();
      //*/
      return (this->state & state_flag::ready) != 0;
   }
   return false;
}
bool DovahKitAssetReceptor::isFailed() const noexcept {
   if (this->asset) {
      return (this->state & state_flag::failed) != 0;
   }
   return false;
}

DovahKitAssetReceptor::DovahKitAssetReceptor(DovahKitAssetTransport&& target) {
   auto* p = target.value;
   target.value = nullptr;
   this->_acquire(p);
}
DovahKitAssetReceptor::DovahKitAssetReceptor(const DovahKitAssetReceptor& other) {
   this->flags = other.flags;
   this->_acquire(other.asset);
}
DovahKitAssetReceptor::DovahKitAssetReceptor(DovahKitAssetReceptor&& other) noexcept {
   this->flags = other.flags;
   this->_acquire(other.asset);
   other._clear();
}
DovahKitAssetReceptor::~DovahKitAssetReceptor() {
}

DovahKitAssetReceptor& DovahKitAssetReceptor::operator=(DovahKitAssetTransport&& target) noexcept {
   if (this->asset == target.value)
      return *this;
   auto* p = target.value;
   target.value = nullptr;
   //
   this->_clear();
   this->_acquire(p);
   return *this;
}
DovahKitAssetReceptor& DovahKitAssetReceptor::operator=(const DovahKitAssetReceptor& other) noexcept {
   this->flags = other.flags;
   this->_clear();
   if (this->asset != other.asset)
      this->_acquire(other.asset);
   return *this;
}
DovahKitAssetReceptor& DovahKitAssetReceptor::operator=(DovahKitAssetReceptor&& other) noexcept {
   this->flags = other.flags;
   this->_clear();
   if (this->asset != other.asset)
      this->_acquire(other.asset);
   other._clear();
   return *this;
}
#pragma endregion