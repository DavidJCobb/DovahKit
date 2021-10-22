#pragma once
#include <QHash>
#include <QObject>
#include <QString>
#include "asset.h"

class DovahKitAsset;

class DovahKitAssetManager : public QObject {
   Q_OBJECT;
   protected:
      DovahKitAssetManager();

      QHash<QString, DovahKitAsset*> assets;
      QVector<QThread> workers;

   public:
      inline static DovahKitAssetManager& get() {
         static DovahKitAssetManager instance;
         return instance;
      }

      // Convert the path to lowercase and normalize directory spearators. Input path should be 
      // relative to the Data directory, and will remain so.
      static QString normalizeAssetPath(const QString&);

      DovahKitAssetHandle loadModel(const QString& path);
      DovahKitAssetHandle loadTexture(const QString& path);
};