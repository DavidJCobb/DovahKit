#pragma once
#include <array>
#include <thread>
#include <QHash>
#include <QObject>
#include <QString>
#include "../../helpers/passkey.h"
#include "asset.h"

class DovahKitAsset;

class DovahKitAssetManager : public QObject {
   Q_OBJECT;
   public:
      static constexpr size_t worker_thread_count = 4;
   protected:
      DovahKitAssetManager();

      class Worker {
         friend class DovahKitAssetManager;
         protected:
            std::thread thread;
            //
            bool termination_requested = false;
            std::atomic<bool> signal = false; // set to (true) to wake the worker when queuing new items, or to terminate it
            struct {
               std::mutex mutex;
               QVector<DovahKitAsset*> to_load;
               QVector<DovahKitAsset*> to_unload;
            } queue;

            void _handler();
            
         public:
            void queueToLoad(DovahKitAsset&);
            void queueToUnload(DovahKitAsset&);

            void start();
            void stop();
            void cancelAll();

            inline bool isRunning() const noexcept { return this->thread.joinable(); }
      };

      QHash<QString, DovahKitAsset*> assets;
      std::array<Worker, worker_thread_count> workers;

   public:
      inline static DovahKitAssetManager& get() {
         static DovahKitAssetManager instance;
         return instance;
      }

      // Convert the path to lowercase and normalize directory spearators. Input path should be 
      // relative to the Data directory, and will remain so.
      static QString normalizeAssetPath(const QString&);

      DovahKitAsset* requestModel(const QString& path);
      DovahKitAsset* requestTexture(const QString& path);

      void load(DovahKitAsset&);

      void onUnreferenced(cobb::passkey<DovahKitAsset, DovahKitAssetManager>, DovahKitAsset&);

   public slots:
      void pauseFormManagement(); // the asset manager won't (un)load forms while this is paused (except when a form deletion is imminent); any such operations will be queued. necessary to avoid thread-safety issues while Dovahscript is active.
      void unpauseFormManagement();

      void unloadAll(); // unloads all assets, forms, etc.. use when the editor as a whole is abandoning a load order and its data.

      void killThreads();  // shuts down all worker threads
      void spawnThreads(); // starts all worker threads back up
};