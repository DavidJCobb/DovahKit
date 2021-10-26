#include "asset_manager.h"
#include <QDir>
#include <QRegularExpression>
#include <QStringView>
#include "../../helpers/cpuinfo.h"
#include "../../dovahscript/dovahscript_host.h"
#include "../../editor/core.h"

#pragma region DovahKitAssetManager::Worker
void DovahKitAssetManager::Worker::_handler() {
   while (true) {
      if (this->termination_requested)
         return;
      //
      auto guard = std::unique_lock(this->queue.mutex);
      for (auto* asset : this->queue.to_load) {
         asset->load();
         if (this->termination_requested)
            return;
      }
      this->queue.to_load.clear();
      for (auto* asset : this->queue.to_unload) {
         asset->unload();
         if (this->termination_requested)
            return;
      }
      this->queue.to_unload.clear();
      //
      if (this->termination_requested)
         return;
      this->signal.wait(false);
      this->signal = false;
   }
}

void DovahKitAssetManager::Worker::queueToLoad(DovahKitAsset& asset) {
   auto guard = std::unique_lock(this->queue.mutex);
   //
   auto& list    = this->queue.to_load;
   auto& inverse = this->queue.to_unload;
   if (list.contains(&asset))
      return;
   inverse.removeOne(&asset);
   list.push_back(&asset);
   //
   this->signal = true;
   this->signal.notify_one();
}
void DovahKitAssetManager::Worker::queueToUnload(DovahKitAsset& asset) {
   auto guard = std::unique_lock(this->queue.mutex);
   //
   auto& list    = this->queue.to_unload;
   auto& inverse = this->queue.to_load;
   if (list.contains(&asset))
      return;
   inverse.removeOne(&asset);
   list.push_back(&asset);
   //
   this->signal = true;
   this->signal.notify_one();
}

void DovahKitAssetManager::Worker::start() {
   this->thread = std::thread(&Worker::_handler, this);
}
void DovahKitAssetManager::Worker::stop() {
   if (!this->thread.joinable())
      return;
   this->termination_requested = true;
   this->signal = true;
   this->signal.notify_one();
   this->thread.join();
   this->termination_requested = false;
}
#pragma endregion

DovahKitAssetManager::DovahKitAssetManager() {
   auto& dovahscript_host = DovahscriptHost::get();
   QObject::connect(&dovahscript_host, &DovahscriptHost::scriptStartImminent, this, &DovahKitAssetManager::pauseFormManagement);
   QObject::connect(&dovahscript_host, &DovahscriptHost::scriptEnded,         this, &DovahKitAssetManager::unpauseFormManagement);
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

DovahKitAsset* DovahKitAssetManager::requestModel(const QString& path) {
   auto norm = this->normalizeAssetPath(path);
   auto it   = this->assets.find(norm);
   if (it != this->assets.end()) {
      return *it;
   }
   auto* asset = new DovahKitAsset(norm, DovahKitAsset::Type::NIF);
   this->assets[norm] = asset;
   this->load(*asset);
   return asset;
}
DovahKitAsset* DovahKitAssetManager::requestTexture(const QString& path) {
   auto norm = this->normalizeAssetPath(path);
   auto it   = this->assets.find(norm);
   if (it != this->assets.end()) {
      return *it;
   }
   auto* asset = new DovahKitAsset(norm, DovahKitAsset::Type::DDS);
   this->assets[norm] = asset;
   this->load(*asset);
   return asset;
}

void DovahKitAssetManager::load(DovahKitAsset& asset) {
   /*// Disabled for now, for simplicity
   //
   size_t lowest = std::numeric_limits<size_t>::max();
   size_t index  = 0;
   //
   {
      std::array<std::unique_lock<std::mutex>, worker_thread_count> guards;
      for (size_t i = 0; i < worker_thread_count; ++i) {
         auto& worker = this->workers[i];
         guards[i] = std::unique_lock(worker.queue.mutex);
         //
         auto s = worker.queue.to_load.size();
         if (s < lowest) {
            lowest = s;
            index  = i;
         }
      }
   }
   //
   this->workers[index].queueToLoad(asset);
   //
   //*/
   asset.load(); // single-threaded load, for testing
}

void DovahKitAssetManager::onUnreferenced(cobb::passkey<DovahKitAsset, DovahKitAssetManager>, DovahKitAsset& asset) {
   auto i = this->assets.remove(asset._path);
   assert(i && "Was this asset not tracked?!");
   delete &asset;
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
   // TODO
}

void DovahKitAssetManager::killThreads() {
   // TODO
}
void DovahKitAssetManager::spawnThreads() {
   // TODO
}