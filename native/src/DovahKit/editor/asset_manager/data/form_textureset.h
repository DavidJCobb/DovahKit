#pragma once
#include <array>
#include <QString>
#include "_base.h"
#include "../../../dovah/form_stub.h"
#include "../asset.h"

namespace dovah {
   namespace loaded_forms {
      class TextureSet;
   }
}

class DovahKitAssetDataTextureSet: public DovahKitAssetData {
   public:
      using DovahKitAssetData::DovahKitAssetData;
      DovahKitAssetDataTextureSet(DovahKitAsset& owner);

      dovah::loaded_form_ptr<dovah::loaded_forms::TextureSet> loaded;
      std::array<DovahKitAssetReceptor, 8> textures = {};

      virtual bool load(const dovah::bsa_archived_file&) override { return false; }
      virtual bool load(dovah::form_stub&) override;

      virtual void abandonDependencies() override;
};