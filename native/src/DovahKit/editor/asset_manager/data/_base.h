#pragma once
#include <atomic>
#include <type_traits>
#include <QString>
#include "../asset.h"
#include "../asset_manager.h"

namespace dovah {
   class bsa_archived_file;
   class form_stub;
}
class DovahKitAsset;

class DovahKitAssetData {
   public:
      DovahKitAssetData(DovahKitAsset& o) : owner(o) {}
      virtual ~DovahKitAssetData() {}

      DovahKitAsset& owner;
      std::atomic<size_t> pending_count = 0;

      virtual bool load(const dovah::bsa_archived_file&) = 0;
      virtual bool load(dovah::form_stub&) = 0;

      virtual bool hasPendingDependencies() const { return this->pending_count > 0; }
      virtual void abandonDependencies() {}

   protected:
      void dependencyResolved();
};
