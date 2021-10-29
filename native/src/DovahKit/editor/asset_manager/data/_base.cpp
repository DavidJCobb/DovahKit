#include "_base.h"
#include "../asset.h"

void DovahKitAssetData::dependencyResolved() {
   --this->pending_count;
   if (this->pending_count == 0)
      emit this->owner.ready();
}