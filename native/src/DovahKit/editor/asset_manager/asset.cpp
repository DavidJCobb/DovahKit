#include "asset.h"
#include "../../../DirectXTex/DirectXTex.h"
#include "../../helpers/intrusive_windows_defines.h"
#include "../../dovah/files/bsa/bsa_archived_file.h"
#include "../core.h"
#include "asset_manager.h"

#include "data/dds.h"
#include "data/form_textureset.h"

namespace {
   // see also: the same constexpr value in asset_manager.cpp
   static constexpr bool debug_asset_lifetime = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;
}

namespace {
   using passkey_to_manager = DovahKitAssetManager::asset_passkey;
}

#pragma region DovahKitAsset
DovahKitAsset::DovahKitAsset(QString p, Type t, QObject* parent) : QObject(parent), _type(t), _path(p) {
   switch (t) {
      case Type::DDS:
         this->data = new DovahKitAssetDataDDS(*this);
         break;
   }
}
DovahKitAsset::DovahKitAsset(dovah::form_stub* s, QObject* parent) : QObject(parent), _type(Type::Form), _stub(s) {
   if (s) {
      switch (s->formType) {
         case dovah::form_type::texture_set:
            this->data = new DovahKitAssetDataTextureSet(*this);
            break;
      }
   }
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

QString DovahKitAsset::description() const noexcept {
   if (this->type() != Type::Form)
      return QString("(%1)").arg(this->_path);
   if (this->_stub) {
      return QString("[FORM:%1]%2").arg(QString("%1").arg(this->_stub->formID, 8, 16, QChar('0'))).arg(this->_stub->editorID.c_str());
   }
   return QString("(?)");
}

const DovahKitAssetDataDDS* DovahKitAsset::asDDS() const noexcept {
   if (this->type() != Type::DDS)
      return nullptr;
   return (DovahKitAssetDataDDS*)this->data;
}
const DovahKitAssetDataTextureSet* DovahKitAsset::asTextureSet() const noexcept {
   if (this->type() != Type::Form)
      return nullptr;
   if (!this->_stub || this->_stub->formType != dovah::form_type::texture_set)
      return nullptr;
   return (DovahKitAssetDataTextureSet*)this->data;
}
//
QImage DovahKitAsset::asQImage() const noexcept {
   if (auto* data = this->asDDS()) {
      return data->image;
   }
   return QImage();
}

void DovahKitAsset::load() {
   auto _fail = [this]() {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset failed to load: %p %s", this, qUtf8Printable(this->description()));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.load_requested = false;
      this->_state.content_failed = true;
      emit contentLoadingFailed();
   };
   auto _done = [this]() {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset loaded: %p %s", this, qUtf8Printable(this->description()));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.load_requested      = false;
      this->_state.content_loaded      = true;
      this->_state.dependencies_loaded = !this->data->hasPendingDependencies();
      emit this->contentLoaded();
      if (this->_state.dependencies_loaded) {
         emit this->ready();
      } else {
         DovahKitAssetManager::get().requestDependencies(passkey_to_manager(), *this);
      }
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
   if (!this->data)
      return _fail();
   //
   if (this->_type == Type::Form) {
      if (!this->_stub)
         return _fail();
      bool result = this->data->load(*this->_stub);
      if (!result)
         return _fail();
      return _done();
   }
   //
   if (this->_path.isEmpty())
      return _fail();
   std::unique_ptr<file_type> file;
   {
      std::filesystem::path std_path = this->_path.toStdWString();
      file.reset(DovahKitCore::get().lookup_game_asset(std_path, true));
   }
   if (!file)
      return _fail();
   //
   // Type-specific loading code:
   //
   bool result = this->data->load(*file);
   if (!result)
      return _fail();
   return _done();
}
void DovahKitAsset::unload() {
   if constexpr (debug_asset_lifetime) {
      qDebug("Unloading DovahKitAsset: %p %s", this, qUtf8Printable(this->description()));
   }
   auto guard = std::unique_lock(this->_locks.load_state);
   this->_state.content_failed      = false;
   this->_state.content_loaded      = false;
   this->_state.dependencies_loaded = false;
   //
   if (auto*& p = this->data) {
      delete p;
      p = nullptr;
   }
}
void DovahKitAsset::requestDependencies() {
   if (!this->data)
      return;
   this->data->requestDependencies();
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
               qDebug("DovahKitAssetReceptor is listening for \"ready\" and \"failed\" signals: %p %s", this, qUtf8Printable(this->asset->description()));
            }
            //
            // The asset is neither already loaded nor already failed, so let's hook signals to it so 
            // that when it loads or fails, we can forward  those signals to whatever owns this asset 
            // receptor. We'll use QueuedConnections here. Technically, we don't need to specify that 
            // at all, because assets (typically) load on  a worker thread and so would automatically 
            // fall back to queued signals. However, form-assets load on the main thread by necessity 
            // and I would prefer that those behave  consistently with other assets. (Plus, requiring 
            // a QueuedConnection saves Qt the trouble of having to check what thread we're on in the 
            // other cases.)
            //
            QObject::connect(asset, &value_type::ready, this, &DovahKitAssetReceptor::_forwardReady, Qt::QueuedConnection);
            QObject::connect(asset, &value_type::contentLoadingFailed, this, &DovahKitAssetReceptor::_forwardFailed, Qt::QueuedConnection);
         }
         QObject::connect(asset, &QObject::destroyed, this, &DovahKitAssetReceptor::_forwardUnloaded, Qt::DirectConnection);
      }
      this->asset->on_handle_made(*this);
   }
   switch (fire) {
      //
      // If the asset is already loaded or failed, we want to re-fire the appropriate signals on this 
      // receptor. The straightforward way would just be to emit the signals normally, but this leads 
      // to inconsistent  behavior with the above (i.e. DirectConnection behavior),  and so can cause 
      // confusing mistakes and issues within outside code which uses asset receptors. Instead, we'll 
      // use Qt's QMetaObject system to emit the signals as a QueuedConnection call.
      //
      case which::ready:
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAssetReceptor is forwarding \"ready\" signal (on acquire): %p %s", this, qUtf8Printable(this->asset->description()));
         }
         QMetaObject::invokeMethod(this, &DovahKitAssetReceptor::ready, Qt::QueuedConnection); // emit the signal, but force it to behave as queued, for uniformity with the above case
         break;
      case which::failed:
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAssetReceptor is forwarding \"failed\" signal (on acquire): %p %s", this, qUtf8Printable(this->asset->description()));
         }
         QMetaObject::invokeMethod(this, &DovahKitAssetReceptor::failed, Qt::QueuedConnection); // emit the signal, but force it to behave as queued, for uniformity with the above case
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
         qDebug("DovahKitAssetReceptor is forwarding \"failed\" signal (after listening): %p %s", this, qUtf8Printable(this->asset->description()));
      }
   }
   emit this->failed();
}
void DovahKitAssetReceptor::_forwardReady() {
   if (this->asset) {
      this->_severLoadSignals();
      this->state |= state_flag::ready;
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAssetReceptor is forwarding \"ready\" signal (after listening): %p %s", this, qUtf8Printable(this->asset->description()));
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