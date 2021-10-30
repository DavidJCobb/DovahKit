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

      virtual bool load(const dovah::bsa_archived_file&) = 0;
      virtual bool load(dovah::form_stub&) = 0;

      // Return true if any changes to dependencies were detected.
      virtual bool onFormModified() { return false; }

      virtual bool hasPendingDependencies() const { return false; }
      virtual void abandonDependencies() {}
};
