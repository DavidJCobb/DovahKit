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
void DovahKitAsset::_initialize() {
   QObject::connect(this, &DovahKitAsset::dependenciesLoaded, this, &DovahKitAsset::onDependenciesLoaded);
}
DovahKitAsset::DovahKitAsset(QString p, Type t, QObject* parent) : QObject(parent), _type(t), _path(p) {
   this->_initialize();
   switch (t) {
      case Type::DDS:
         this->data = new DovahKitAssetDataDDS(*this);
         break;
   }
}
DovahKitAsset::DovahKitAsset(dovah::form_stub* s, QObject* parent) : QObject(parent), _type(Type::Form), _stub(s) {
   this->_initialize();
   if (s) {
      switch (s->formType) {
         case dovah::form_type::texture_set:
            this->data = new DovahKitAssetDataTextureSet(*this);
            break;
      }
      if (this->data) {
         auto& editor = DovahKitCore::get();
         QObject::connect(&editor, &DovahKitCore::formModified, this, &DovahKitAsset::onFormModified);
      }
   }
}
DovahKitAsset::~DovahKitAsset() {
   if constexpr (debug_asset_lifetime) {
      qDebug("Running DovahKitAsset destructor: %p %s", this, qUtf8Printable(this->description()));
   }
   this->unload();
}

void DovahKitAsset::on_handle_made(DovahKitAssetReceptor& handle) {
   auto to = ++this->_state.refcounts.all;
   if (handle.flags & DovahKitAssetReceptor::Flag::IsRenderWindow)
      ++this->_state.refcounts.render_window;
   //
   if (to == 1 && this->_state.when_unreferenced.isValid())
      DovahKitAssetManager::get().onReReferenced(passkey_to_manager(), *this);
}
void DovahKitAsset::on_handle_lost(DovahKitAssetReceptor& handle) {
   auto to = --this->_state.refcounts.all;
   if (handle.flags & DovahKitAssetReceptor::Flag::IsRenderWindow)
      --this->_state.refcounts.render_window;
   //
   if (to == 0)
      DovahKitAssetManager::get().onUnreferenced(passkey_to_manager(), *this);
}
void DovahKitAsset::on_transport_start(DovahKitAssetTransport& transport) {
   auto to = ++this->_state.refcounts.all;
   //
   if (to == 1 && this->_state.when_unreferenced.isValid())
      DovahKitAssetManager::get().onReReferenced(passkey_to_manager(), *this);
}

bool DovahKitAsset::samePathAs(const QString& p) const noexcept {
   if (this->type() == Type::Form) {
      return false;
   }
   auto np = DovahKitAssetManager::normalizeAssetPath(p);
   return np == this->_path;
}

QString DovahKitAsset::description() const noexcept {
   if (this->type() != Type::Form)
      return QString("(%1)").arg(this->_path);
   if (this->_stub) {
      return QString("[FORM:%1]%2").arg(QString("%1").arg(this->_stub->formID, 8, 16, QChar('0'))).arg(this->_stub->editorID.c_str());
   }
   return QString("[FORM:????????]UnloadedAsset");
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
         qDebug("DovahKitAsset: Failed: %p %s", this, qUtf8Printable(this->description()));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.content_failed = true;
      this->data->abandonDependencies();
      emit contentLoadingFailed();
      this->_state.load_requested = false;
   };
   auto _done = [this]() {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset: Loaded: %p %s", this, qUtf8Printable(this->description()));
      }
      auto guard = std::unique_lock(this->_locks.load_state);
      this->_state.content_loaded      = true;
      this->_state.dependencies_loaded = !this->data->hasPendingDependencies();
      emit this->contentLoaded();
      if (this->_state.dependencies_loaded) {
         emit this->ready();
      }
      this->_state.load_requested = false;
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
      qDebug("DovahKitAsset: Unloading: %p %s", this, qUtf8Printable(this->description()));
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
   //
   // Edge-case: when DovahKit is about to abandon all loaded game data, the DovahKitAssetManager will respond 
   // by mass-unloading all  assets, including form-assets; we call DovahKitAsset::unload  and then delete the 
   // asset. However, we use QObject::deleteLater in order to avoid deleting assets out from under any pending 
   // Qt events...  and that necessarily means that the  form-assets will be deleted after game  data has been 
   // discarded. The stub will not be safe to query (whether  by debug logging or anything else) at that time. 
   // We need to clear it here, in DovahKitAsset::unload.
   //
   this->_stub = nullptr;
}
void DovahKitAsset::onDependenciesLoaded() {
   auto& state = this->_state;
   auto  guard = std::unique_lock(this->_locks.load_state);
   if (state.dependencies_loaded)
      return;
   state.dependencies_loaded = true;
   if (state.content_loaded) {
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAsset: Dependencies loaded; now ready: %p %s", this, qUtf8Printable(this->description()));
      }
      emit this->ready();
   }
}
void DovahKitAsset::onFormModified() {
   if (this->data) {
      bool changed = this->data->onFormModified();
      if (changed) {
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAsset: Detected form modification: %p %s", this, qUtf8Printable(this->description()));
         }
         emit this->dependenciesChanged();
      }
   }
}
#pragma endregion

#pragma region DovahKitAssetTransport
DovahKitAssetTransport::DovahKitAssetTransport(DovahKitAsset* v) : value(v) {
   _inc();
}
DovahKitAssetTransport::~DovahKitAssetTransport() {
   assert(this->value == nullptr && "a DovahKitAssetTransport failed to make it into a receptor");
   _dec();
}

DovahKitAssetTransport::DovahKitAssetTransport(DovahKitAssetTransport&& other) noexcept {
   if (this->value != other.value) {
      _dec();
      this->value = other.value;
   }
   other.value = nullptr;
}
DovahKitAssetTransport& DovahKitAssetTransport::operator=(DovahKitAssetTransport&& other) noexcept {
   if (this->value != other.value) {
      _dec();
      this->value = other.value;
   }
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
         QObject::connect(asset, &value_type::dependenciesChanged, this, &DovahKitAssetReceptor::_forwardDependenciesChanged, Qt::DirectConnection);
         QObject::connect(asset, &QObject::destroyed,              this, &DovahKitAssetReceptor::_forwardUnloaded,            Qt::DirectConnection);
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

#pragma region Member functions to forward signals
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
void DovahKitAssetReceptor::_forwardDependenciesChanged() {
   emit this->dependenciesChanged();
}
#pragma endregion

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
   this->_acquire(p);
   target._clear();
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
   this->_clear();
}

DovahKitAssetReceptor& DovahKitAssetReceptor::operator=(DovahKitAssetTransport&& target) noexcept {
   auto* p = target.value;
   if (this->asset != p) {
      this->_clear();
      this->_acquire(p);
   }
   target._clear();
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