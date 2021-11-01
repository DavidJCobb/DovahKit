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

   //
   // Delay before unloading an unreferenced asset, in milliseconds.
   //
   static constexpr uint deferred_unload_time = 500; // controls how we schedule timer checks on assets
   static constexpr uint deferred_unload_fuzz = 100; // "fuzz" factor; we will unload assets that are within this many milliseconds of the desired unload time

   //
   // Assets which become unreferenced are added  to a queue of assets awaiting deferred 
   // unloading. This queue is processed on a timer: we loop over its contents, and each 
   // asset which has been unreferenced for  roughly (deferred_unload_time) milliseconds 
   // will be discarded. Of course, we have to lock the list of all extant assets before 
   // we can safely discard any assets, lest requestAsset allow another thread to access 
   // the asset while we're discarding it.
   // 
   // There are  two approaches to locking that  list: do it for the full time  we spend 
   // looping over the deferred-unload assets; or only lock upon finding an asset that's 
   // actually ready for deletion (i.e. it's been unreferenced for long enough). The 
   // approach that we take will have broader implications.
   //
   static constexpr bool deferred_unload_does_last_second_locking = false;
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
   //
   {
      auto& du = this->deferred_unload;
      //
      du.timer.setSingleShot(true);
      if constexpr (deferred_unload_time > 0) {
         QObject::connect(&du.timer, &QTimer::timeout, this, &DovahKitAssetManager::onDeferredUnloadTimer);
      }
   }
}
DovahKitAssetManager::~DovahKitAssetManager() {
   this->unloadAll();
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
void DovahKitAssetManager::_discard(DovahKitAsset& asset, bool lock) {
   using guard_type = std::unique_lock<decltype(this->asset_lock)>; // default-construction doesn't pass an argument to infer the template type from, so we need to manually specify that
   {
      auto guard = lock ? guard_type(this->asset_lock) : guard_type();
      if constexpr (deferred_unload_time > 0) {
         //
         // If deferred unloading is enabled, then an asset could plausibly become referenced again 
         // after initially becoming unreferenced and being marked for deferred unloading.
         //
         if (asset._state.refcounts.all > 0)
            return;
      }
      if (asset.type() == DovahKitAsset::Type::Form) {
         if (this->isFormManagementPaused()) {
            auto& list = this->forms.queues.discard;
            this->forms.queues.load.removeOne(&asset);
            if (!list.contains(&asset))
               list.push_back(&asset);
         }
         auto i = this->forms.assets.remove(asset._stub);
         assert(i && "Was this asset not tracked?!");
      } else {
         auto i = this->assets.remove(asset._path);
         assert(i && "Was this asset not tracked?!");
      }
   }
   if (asset._state.load_requested) { // unlocked check for speed; we'll do a locked check if this passes
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

DovahKitAssetTransport DovahKitAssetManager::requestAsset(const QString& path) {
   bool on_same_thread = QThread::currentThread() == this->thread();
   //
   auto norm = this->normalizeAssetPath(path);
   if (norm.isEmpty())
      return nullptr;
   DovahKitAsset* asset = nullptr;
   {
      auto guard = std::unique_lock(this->asset_lock);
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
      auto guard = std::unique_lock(this->asset_lock);
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
   if constexpr (deferred_unload_time > 0) {
      asset._state.when_unreferenced.start();
      //
      auto& du    = this->deferred_unload;
      auto  guard = std::unique_lock(du.lock);
      //
      du.queue.push_back(&asset);
      if (!du.timer.isActive()) {
         if constexpr (debug_asset_lifetime) {
            qDebug("DovahKitAssetManager: Queueing the deferred-unload process to run in %u ms...", deferred_unload_time);
         }
         du.timer.start(deferred_unload_time);
      }
   } else {
      this->_discard(asset);
   }
}
void DovahKitAssetManager::onReReferenced(asset_passkey, DovahKitAsset& asset) {
   if constexpr (debug_asset_lifetime) {
      qDebug("DovahKitAsset became referenced after previously being unreferenced: %p %s", &asset, qUtf8Printable(asset.description()));
   }
   auto& du    = this->deferred_unload;
   auto  guard = std::unique_lock(du.lock, std::try_to_lock);
   //
   if constexpr (deferred_unload_does_last_second_locking) {
      if (!guard.owns_lock())
         //
         // Failed to get the lock;  the queue is actively  being processed. The function 
         // to process the queue will check the asset's refcount anyway, so c'est la vie.
         //
         return;
   } else {
      //
      // The only  circumstance in which we'd fail to grab that lock is if the  deferred-
      // unload process is running.
      //
      // If the deferred-unload process doesn't do last-second locking, then the list of 
      // all assets should be locked during that  process... which means it shouldn't be 
      // possible at  all for a formerly-unreferenced  asset to become  referenced while 
      // that process is ongoing. So let's walk through this:
      // 
      // The requestAsset function would hit the lock, get the lock, and return an asset 
      // wrapped in  a DovahKitAssetTransport; the transport would temporarily  increase 
      // the asset's refcount. The transport would then be captured in a receptor, which 
      // would increase the asset's refcount for its own purposes and then empty out the 
      // transport, decreasing the refcount. The net result of this entire process would 
      // be the refcount increasing by 1 (changes: +1; +1; -1).
      // 
      // Assets can call onReReferenced when they are wrapped in a transport or acquired 
      // by a receptor. This means that in the  above scenario, onReReferenced should be 
      // called  when the transport is created by  requestAsset, before the list  of all 
      // assets is unlocked by requestAsset.  (If transports didn't call onReReferenced, 
      // then there'd be a brief window (between  requestAsset returning a transport and 
      // the transport being received by a receptor) in which we could start running the 
      // deferred-unload  process,  thereby allowing the asset  to become  re-referenced 
      // while that process is running and bringing us here. But we cover that case.)
      // 
      // Ergo it should be friggin' impossible for us to end up... here, at this spot.
      //
      assert(guard.owns_lock() && "How did an asset become re-referenced while assets pending deletion are being processed?");
   }
   asset._state.when_unreferenced.invalidate();
   auto found = du.queue.removeOne(&asset);
   assert(found);
   if (du.queue.isEmpty() && du.timer.isActive())
      du.timer.stop();
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
   auto& du = this->deferred_unload;
   auto guard_a = std::unique_lock(du.lock);
   auto guard_b = std::unique_lock(this->asset_lock);
   du.queue.clear();
   du.timer.stop();
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
void DovahKitAssetManager::onDeferredUnloadTimer() {
   using guard_t = std::unique_lock<decltype(this->asset_lock)>; // default-construction doesn't pass an argument to infer the template type from, so we need to manually specify that
   //
   auto&   du      = this->deferred_unload;
   auto    guard_a = std::unique_lock(du.lock);
   guard_t guard_b;
   if constexpr (deferred_unload_does_last_second_locking) {
      //
      // We need to lock the list of all assets before we make any attempt to delete 
      // an asset; the last thing we want is for an asset to be requested by another 
      // thread and received by that thread, all while we're deleting it.
      // 
      // However, we can decide to lock the asset list preemptively whenever we need 
      // to check for assets that should unload;  or we can decide to lock the asset 
      // list at the last moment,  i.e. when we actually  find an asset that we want 
      // to unload.
      //
      guard_b = std::unique_lock(this->asset_lock, std::defer_lock);
   } else {
      guard_b = std::unique_lock(this->asset_lock);
   }
   //
   // We're going to loop over all assets that have been marked as awaiting a deferred 
   // deletion (a.k.a. deferred unload). Each asset maintains a QElapsedTimer which is 
   // used to mark when the asset became unreferenced; if an asset is unreferenced for 
   // long enough, we delete it.
   // 
   // If, on the other hand, we find an asset that isn't yet ready to be deleted, then 
   // we'll check how many milliseconds must pass until it *is* ready for deletion. We 
   // want to store  the lowest such value, and use that to schedule  the next call to 
   // this function -- the next loop-and-delete.
   // 
   // Formerly-unreferenced assets can only  become referenced again while the list of 
   // all assets is  unlocked; if and only if  that happens while this  function isn't 
   // running, then they'll be removed from  (du.queue) immediately. Assets can become 
   // unreferenced at any time, but cannot be  added to (du.queue) until this function 
   // exits.
   //
   uint lowest_next = deferred_unload_time;
   for (auto*& asset : du.queue) {
      //
      // The (asset) variable is not merely a pointer, but a reference to the specific 
      // pointer in the list. This means that if  we decide to delete an asset, we can 
      // set its entry in the list to nullptr; then, after the loop, we can remove all 
      // nullptr values en masse. Makes for easy cleanup.
      //
      auto& time = asset->_state.when_unreferenced;
      if (!time.isValid())
         continue;
      auto e = time.elapsed();
      if (e >= (deferred_unload_time - deferred_unload_fuzz)) {
         if constexpr (deferred_unload_does_last_second_locking) {
            //
            // If this branch runs, then we've been configured to lock the asset list 
            // only when we actually commit to deleting an asset.
            //
            if (!guard_b.owns_lock()) {
               guard_b.lock();
               if (asset->_state.refcounts.all) {
                  //
                  // If we only lock the list of all assets after deciding to delete at 
                  // least one asset, then it's possible  for another thread to request 
                  // and reference that asset while we're making our decision. As such, 
                  // once we have  the lock we need to double-check that  the asset did 
                  // not become referenced.
                  //
                  asset = nullptr;
                  continue;
               }
            }
         }
         time.invalidate();
         this->_discard(*asset, false);
         asset = nullptr;
      } else {
         if (asset->_state.refcounts.all) { // false-negatives possible if we haven't locked the asset list, but acceptable
            //
            // This asset  became unreferenced at some point and was  marked for a 
            // deferred unload, but then it became referenced again while awaiting 
            // unloading. It wasn't removed from  (du.queue) because that happened 
            // while this function was already running.  Let's remove it from this 
            // list.
            //
            asset = nullptr;
            continue;
         }
         e = deferred_unload_time - e;
         if (e < lowest_next)
            lowest_next = e;
      }
   }
   if constexpr (deferred_unload_does_last_second_locking) {
      if (guard_b.owns_lock())
         guard_b.unlock();
   } else {
      guard_b.unlock();
   }
   //
   // Now let's remove all nullptr values from  the queue, as described above. It may 
   // be tempting  to put this behind a bool -- to run the "removeAll"  function only 
   // if we discarded an asset in the loop above -- but there's no need. The function 
   // we're in right  now is timed to only run when an asset is ready  to unload, and 
   // that timer is canceled if the asset becomes referenced again.
   //
   auto count = du.queue.removeAll(nullptr);
   if constexpr (debug_asset_lifetime) {
      if (count)
         qDebug("DovahKitAssetManager: %d assets were deleted or skipped (i.e. became re-referenced) by the deferred-unload process.", count);
   }
   //
   if (!du.queue.isEmpty()) {
      //
      // There are still some items pending a deferred unload, so restart our 
      // timer for them.
      //
      if constexpr (debug_asset_lifetime) {
         qDebug("DovahKitAssetManager: Queueing the deferred-unload process to run again in %u ms...", lowest_next);
      }
      du.timer.start(lowest_next);
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