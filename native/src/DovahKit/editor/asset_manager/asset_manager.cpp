#include "asset_manager.h"
#include <QDir>
#include <QRegularExpression>
#include <QStringView>
#include <QThread>
#include "../../helpers/cpuinfo.h"
#include "../../dovahscript/dovahscript_host.h"
#include "../../editor/core.h"

// for DirectXTex and COM setup:
#include <windows.h>
#include "../../helpers/intrusive_windows_defines.h"

namespace {
   // see also: the same constexpr value in asset.cpp
   static constexpr bool debug_asset_lifetime = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;

   static constexpr bool debug_worker_threads = false
      #ifdef _DEBUG
         || _DEBUG
      #endif
   ;

   static constexpr bool force_loading_on_main_thread = false;
}

#pragma region DovahKitAssetManager::Worker
void DovahKitAssetManager::Worker::_handler() {
   {
      //
      // Set up COM on this thread so that it can use DirectXTex.
      //
      HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
      if (!FAILED(hr)) {
         this->com_is_ready = true;
      }
   }
   while (true) {
      if (this->termination_requested)
         break;
      {
         decltype(this->queue.list) local;
         {
            auto guard = std::unique_lock(this->queue.mutex);
            std::swap(local, this->queue.list);
         }
         for (auto* asset : local) {
            asset->load();
            if (this->termination_requested)
               break;
         }
      }
      if (this->termination_requested)
         break;
      this->signal.wait(false);
      this->signal = false;
      //
      if constexpr (debug_worker_threads) {
         qDebug("Asset manager thread woke up: %08X", this->thread.get_id());
      }
   }
   if (this->com_is_ready) {
      CoUninitialize(); // every CoInitializeEx call must have a matching CoUninitialize call
      this->com_is_ready = false;
   }
}

void DovahKitAssetManager::Worker::queueToLoad(DovahKitAsset& asset) {
   auto guard = std::unique_lock(this->queue.mutex);
   //
   auto& list = this->queue.list;
   if (list.contains(&asset))
      return;
   list.push_back(&asset);
   //
   this->signal = true;
   this->signal.notify_one();
}

void DovahKitAssetManager::Worker::start() {
   this->thread = std::thread(&Worker::_handler, this);
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager thread starting: %08X", this->thread.get_id());
   }
}
void DovahKitAssetManager::Worker::stop() {
   if (!this->thread.joinable())
      return;
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager thread stopping: %08X", this->thread.get_id());
   }
   this->termination_requested = true;
   this->signal = true;
   this->signal.notify_one();
   this->thread.join();
   this->termination_requested = false;
   this->signal = false;
   {
      auto guard = std::unique_lock(this->queue.mutex);
      this->queue.list.clear();
   }
}
#pragma endregion

DovahKitAssetManager::DovahKitAssetManager() {
   auto& dovahscript_host = DovahscriptHost::get();
   QObject::connect(&dovahscript_host, &DovahscriptHost::scriptStartImminent, this, &DovahKitAssetManager::pauseFormManagement);
   QObject::connect(&dovahscript_host, &DovahscriptHost::scriptEnded,         this, &DovahKitAssetManager::unpauseFormManagement);
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,  this, &DovahKitAssetManager::spawnThreads);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,  this, &DovahKitAssetManager::unloadAll);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, &DovahKitAssetManager::onFormDeleted);
}

/*static*/ QString DovahKitAssetManager::normalizeAssetPath(const QString& base) {
   auto path = QDir::cleanPath(base);
   if (path.isEmpty() || path.startsWith("../"))
      return path;
   if (path.startsWith('/'))
      path = path.mid(1);
   path = path.toLower();
   return path;
}

void DovahKitAssetManager::_load(DovahKitAsset& asset) {
   assert(asset.type() != DovahKitAsset::Type::Form);
   {
      auto guard = std::unique_lock(asset._locks.load_state);
      if (asset._state.load_requested)
         return;
      asset._state.load_requested = true;
   }
   if constexpr (debug_asset_lifetime) {
      qDebug("DovahKitAsset asset load requested from manager: %p %s", &asset, qUtf8Printable(asset.description()));
   }
   if constexpr (force_loading_on_main_thread) {
      asset.load();
   } else {
      auto& which = this->last_worker;
      which = (which + 1) % worker_thread_count;
      this->workers[which].queueToLoad(asset);
   }
}

DovahKitAssetTransport DovahKitAssetManager::requestAsset(const QString& path) {
   bool on_same_thread = QThread::currentThread() == this->thread();
   //
   auto norm = this->normalizeAssetPath(path);
   if (norm.isEmpty())
      return nullptr;
   DovahKitAsset* asset = nullptr;
   {
      auto guard = std::shared_lock(this->asset_lock);
      auto it    = this->assets.find(norm);
      if (it != this->assets.end()) {
         return *it;
      }
      auto type = DovahKitAsset::Type::Undefined;
      if (path.endsWith(".dds"))
         type = DovahKitAsset::Type::DDS;
      else if (path.endsWith(".dds"))
         type = DovahKitAsset::Type::NIF;
      //
      asset = new DovahKitAsset(norm, type);
      this->assets[norm] = asset;
      if (!on_same_thread) {
         asset->moveToThread(this->thread());
      }
   }
   if constexpr (debug_asset_lifetime) {
      qDebug("Created DovahKitAsset: %p %s", asset, qUtf8Printable(asset->description()));
   }
   this->_load(*asset);
   return asset;
}
DovahKitAssetTransport DovahKitAssetManager::requestAsset(dovah::form_stub& stub) {
   bool on_same_thread = QThread::currentThread() == this->thread();
   //
   DovahKitAsset* asset = nullptr;
   {
      auto guard = std::shared_lock(this->asset_lock);
      auto it    = this->forms.assets.find(&stub);
      if (it != this->forms.assets.end()) {
         return *it;
      }
      asset = new DovahKitAsset(&stub);
      this->forms.assets[&stub] = asset;
      if (!on_same_thread) {
         asset->moveToThread(this->thread());
      }
   }
   if constexpr (debug_asset_lifetime) {
      qDebug("Created DovahKitAsset: %p %s", asset, qUtf8Printable(asset->description()));
   }
   if (this->isFormManagementPaused()) {
      auto guard = std::unique_lock(this->asset_lock);
      this->forms.queues.load.push_back(asset);
   } else {
      if (on_same_thread) {
         asset->load(); // MUST occur on this thread
      } else {
         QMetaObject::invokeMethod(this, "loadFormAsset", Qt::QueuedConnection, Q_ARG(DovahKitAsset*, asset)); // the function pointer overload doesn't support arguments. nice one, Qt
      }
   }
   return asset;
}

void DovahKitAssetManager::onUnreferenced(asset_passkey, DovahKitAsset& asset) {
   if constexpr (debug_asset_lifetime) {
      qDebug("DovahKitAsset is unreferenced: %p %s", &asset, qUtf8Printable(asset.description()));
   }
   if (asset.type() == DovahKitAsset::Type::Form && this->isFormManagementPaused()) {
      auto  guard = std::unique_lock(this->asset_lock);
      auto& list  = this->forms.queues.discard;
      this->forms.queues.load.removeOne(&asset);
      if (!list.contains(&asset))
         list.push_back(&asset);
      return;
   }
   {
      auto guard = std::unique_lock(this->asset_lock);
      if (asset.type() == DovahKitAsset::Type::Form) {
         auto i = this->forms.assets.remove(asset._stub);
         assert(i && "Was this asset not tracked?!");
      } else {
         auto i = this->assets.remove(asset._path);
         assert(i && "Was this asset not tracked?!");
      }
   }
   if (asset._state.load_requested) {
      //
      // It's not safe to delete assets while they're queued to load.
      //
      auto guard = std::unique_lock(asset._locks.load_state);
      if (asset._state.load_requested) {
         QObject::connect(&asset, &DovahKitAsset::contentLoaded,        &asset, &QObject::deleteLater);
         QObject::connect(&asset, &DovahKitAsset::contentLoadingFailed, &asset, &QObject::deleteLater);
         return;
      }
   }
   asset.deleteLater();
}

void DovahKitAssetManager::pauseFormManagement() {
   assert(QThread::currentThread() == this->thread());
   //
   this->forms.paused = true;
}
void DovahKitAssetManager::unpauseFormManagement() {
   assert(QThread::currentThread() == this->thread());
   //
   decltype(this->forms.queues.load)    lq;
   decltype(this->forms.queues.discard) dq;
   {
      auto guard = std::unique_lock(this->asset_lock);
      this->forms.paused = false;
      std::swap(lq, this->forms.queues.load);
      std::swap(dq, this->forms.queues.discard);
   }
   for (auto* asset : lq)
      asset->load();
   for (auto* asset : dq)
      asset->deleteLater();
}

void DovahKitAssetManager::unloadAll() {
   assert(QThread::currentThread() == this->thread());
   //
   if constexpr (debug_asset_lifetime || debug_worker_threads) {
      qDebug("Asset manager is unloading all content and killing all worker threads...");
   }
   this->killThreads();
   //
   auto guard = std::unique_lock(this->asset_lock);
   {
      for (auto* asset : this->assets) {
         //
         // See below.
         //
         asset->unload();
         asset->deleteLater();
      }
      this->assets.clear();
   }
   {
      for (auto* asset : this->forms.assets) {
         //
         // The asset destructor unloads the asset content, but because we're calling deleteLater, 
         // that  may not happen soon enough. Commonly,  we would want to mass-unload assets  when 
         // DovahKit is about to discard all loaded data... and for form-assets, we need to unload 
         // data IMMEDIATELY or we'll cause issues with deleting the form stubs.
         // 
         // We'll unload now, and also call deleteLater.
         //
         asset->unload();
         asset->deleteLater();
      }
      this->forms.assets.clear();
   }
   this->forms.queues.load.clear();
   this->forms.queues.discard.clear();
}

void DovahKitAssetManager::killThreads() {
   assert(QThread::currentThread() == this->thread());
   //
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager is killing all worker threads...");
   }
   for (auto& worker : this->workers)
      worker.stop();
}
void DovahKitAssetManager::spawnThreads() {
   assert(QThread::currentThread() == this->thread());
   //
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager is spawning all worker threads...");
   }
   assert(!this->workers[0].isRunning());
   for (auto& worker : this->workers)
      worker.start();
}

void DovahKitAssetManager::onFormDeleted(dovah::form_stub* stub, bool will_be_flagged) {
   auto guard = std::unique_lock(this->asset_lock);
   auto it    = this->forms.assets.find(stub);
   if (it != this->forms.assets.end()) {
      (*it)->unload();
      (*it)->deleteLater();
      this->forms.assets.erase(it);
   }
   //
   if (this->isFormManagementPaused()) {
      int i = -1;
      for (int j = 0; j < this->forms.queues.load.size(); ++j) {
         auto* asset = this->forms.queues.load[j];
         if (asset->_stub == stub) {
            // don't call deleteLater here; assets in this queue are also in this->forms.assets, so it will have been called above
            i = j;
            break;
         }
      }
      if (i >= 0)
         this->forms.queues.load.removeAt(i);
      //
      i = -1;
      for (int j = 0; j < this->forms.queues.discard.size(); ++j) {
         auto* asset = this->forms.queues.discard[j];
         if (asset->_stub == stub) {
            // don't call deleteLater here; assets in this queue are also in this->forms.assets, so it will have been called above
            i = j;
            break;
         }
      }
      if (i >= 0)
         this->forms.queues.load.removeAt(i);
   }
}
void DovahKitAssetManager::loadFormAsset(DovahKitAsset* asset) {
   assert(asset);
   assert(QThread::currentThread() == this->thread() && "This function should only run on the main thread; its literal sole purpose is to carry out a main-thread form-asset load.");
   //
   if (this->isFormManagementPaused()) {
      auto guard = std::unique_lock(this->asset_lock);
      this->forms.queues.load.push_back(asset);
      return;
   }
   asset->load();
}