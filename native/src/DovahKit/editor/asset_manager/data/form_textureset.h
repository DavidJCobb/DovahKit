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
   protected:
      struct path_list {
         std::array<QString, 8> paths = {};
      };

      path_list* pending       = nullptr;
      size_t     pending_count = 0;

   public:
      using DovahKitAssetData::DovahKitAssetData;

      dovah::loaded_form_ptr<dovah::loaded_forms::TextureSet> loaded;
      std::array<DovahKitAssetReceptor, 8> textures = {};

      virtual bool load(const dovah::bsa_archived_file&) override { return false; }
      virtual bool load(dovah::form_stub&) override;

      virtual bool hasPendingDependencies() const override;
      virtual void requestDependencies() override;

      static_assert(std::tuple_size_v<decltype(path_list::paths)> == std::tuple_size_v<decltype(textures)>);
};