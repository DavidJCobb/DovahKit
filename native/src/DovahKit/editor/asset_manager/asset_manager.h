#pragma once
#include <array>
#include <atomic>
#include <shared_mutex>
#include <thread>
#include <QHash>
#include <QObject>
#include <QString>
#include <QTimer>
#include "../../helpers/passkey.h"
#include "asset.h"

namespace dovah {
   class form_stub;
}
class DovahKitAsset;

class DovahKitAssetManager : public QObject {
   Q_OBJECT;
   public:
      static constexpr size_t worker_thread_count = 4;

      using asset_passkey = cobb::passkey<DovahKitAsset, DovahKitAssetManager>;

   protected:
      DovahKitAssetManager();
      ~DovahKitAssetManager();

      class Worker {
         friend class DovahKitAssetManager;
         protected:
            std::thread thread;
            //
            bool termination_requested = false;
            std::atomic<bool> signal = false; // set to (true) to wake the worker when queuing new items, or to terminate it
            struct {
               std::mutex mutex;
               QVector<DovahKitAsset*> list;
            } queue;
            //
            bool com_is_ready = false; // needed for DirectXTex

            void _handler();
            
         public:
            void queueToLoad(DovahKitAsset&);

            void start();
            void stop();
            void cancelAll();

            inline bool isRunning() const noexcept { return this->thread.joinable(); }
      };

      std::mutex asset_lock;
      QHash<QString, DovahKitAsset*> assets;
      struct {
         QHash<dovah::form_stub*, DovahKitAsset*> assets;
         struct {
            QVector<DovahKitAsset*> load;
            QVector<DovahKitAsset*> discard;
         } queues;
         std::atomic<bool> paused = false;
      } forms;
      struct {
         std::mutex lock;
         QVector<DovahKitAsset*> queue;
         QTimer timer;
      } deferred_unload;
      std::atomic<bool> teardown_in_progress = false;
      //
      std::array<Worker, worker_thread_count> workers;
      size_t last_worker = worker_thread_count - 1;

      void _load(DovahKitAsset&);
      void _discard(DovahKitAsset&, bool lock = true);

   public:
      inline static DovahKitAssetManager& get() {
         static DovahKitAssetManager instance;
         return instance;
      }

      // Convert the path to lowercase and normalize directory spearators. Input path should be 
      // relative to the Data directory, and will remain so.
      static QString normalizeAssetPath(const QString&);

      DovahKitAssetTransport requestAsset(const QString& path);
      DovahKitAssetTransport requestAsset(dovah::form_stub& stub);

      void onUnreferenced(asset_passkey, DovahKitAsset&);
      void onReReferenced(asset_passkey, DovahKitAsset&);

      inline bool isFormManagementPaused() const noexcept { return this->forms.paused; }

   public slots:
      void pauseFormManagement(); // the asset manager won't (un)load forms while this is paused (except when a form deletion is imminent); any such operations will be queued. necessary to avoid thread-safety issues while Dovahscript is active.
      void unpauseFormManagement();

      void unloadAll(); // unloads all assets, forms, etc.. use when the editor as a whole is abandoning a load order and its data.

      void killThreads();  // shuts down all worker threads
      void spawnThreads(); // starts all worker threads back up

   protected slots:
      void onFormDeleted(dovah::form_stub*, bool will_be_flagged);
      void onDeferredUnloadTimer();
      void loadFormAsset(DovahKitAsset*);
};