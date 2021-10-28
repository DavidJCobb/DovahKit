#include "asset_manager.h"
#include <QDir>
#include <QRegularExpression>
#include <QStringView>
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
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &DovahKitAssetManager::spawnThreads);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &DovahKitAssetManager::unloadAll);
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
   {
      auto guard = std::unique_lock(asset._locks.load_state);
      if (asset._state.load_requested)
         return;
      asset._state.load_requested = true;
   }
   if constexpr (debug_asset_lifetime) {
      qDebug("DovahKitAsset asset load requested from manager: %p (%s)", &asset, qUtf8Printable(asset._path));
   }
   if constexpr (force_loading_on_main_thread) {
      asset.load();
   } else {
      size_t lowest = std::numeric_limits<size_t>::max();
      size_t index  = 0;
      //
      {
         std::array<std::unique_lock<std::mutex>, worker_thread_count> guards = {};
         for (size_t i = 0; i < worker_thread_count; ++i) {
            auto& worker = this->workers[i];
            guards[i] = std::unique_lock(worker.queue.mutex);
            //
            auto s = worker.queue.list.size();
            if (s < lowest) {
               lowest = s;
               index  = i;
            }
         }
      }
      //
      this->workers[index].queueToLoad(asset);
   }
}

DovahKitAssetTransport DovahKitAssetManager::requestAsset(const QString& path) {
   auto norm = this->normalizeAssetPath(path);
   auto it   = this->assets.find(norm);
   if (it != this->assets.end()) {
      return *it;
   }
   //
   auto type = DovahKitAsset::Type::Undefined;
   if (path.endsWith(".dds"))
      type = DovahKitAsset::Type::DDS;
   else if (path.endsWith(".dds"))
      type = DovahKitAsset::Type::NIF;
   //
   auto* asset = new DovahKitAsset(norm, type);
   this->assets[norm] = asset;
   if constexpr (debug_asset_lifetime) {
      qDebug("Created DovahKitAsset: %p (%s)", asset, qUtf8Printable(path));
   }
   this->_load(*asset);
   return asset;
}

void DovahKitAssetManager::onUnreferenced(cobb::passkey<DovahKitAsset, DovahKitAssetManager>, DovahKitAsset& asset) {
   if constexpr (debug_asset_lifetime) {
      qDebug("DovahKitAsset is unreferenced: %p (%s)", &asset, qUtf8Printable(asset._path));
   }
   auto i = this->assets.remove(asset._path);
   assert(i && "Was this asset not tracked?!");
   asset.deleteLater();
}

void DovahKitAssetManager::pauseFormManagement() {
   // TODO: when we add support for managing whole forms, do not allow the asset manager 
   // to load or unload forms while "form management" is paused, EXCEPT for one thing: we 
   // MUST always unload forms when their deletion is imminent.
}
void DovahKitAssetManager::unpauseFormManagement() {
   // TODO
}

void DovahKitAssetManager::unloadAll() {
   if constexpr (debug_asset_lifetime || debug_worker_threads) {
      qDebug("Asset manager is unloading all content and killing all worker threads...");
   }
   this->killThreads();
   {
      decltype(this->assets) list;
      std::swap(list, this->assets);
      for (auto* asset : list)
         asset->deleteLater();
   }
}

void DovahKitAssetManager::killThreads() {
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager is killing all worker threads...");
   }
   for (auto& worker : this->workers)
      worker.stop();
}
void DovahKitAssetManager::spawnThreads() {
   if constexpr (debug_worker_threads) {
      qDebug("Asset manager is spawning all worker threads...");
   }
   assert(!this->workers[0].isRunning());
   for (auto& worker : this->workers)
      worker.start();
}