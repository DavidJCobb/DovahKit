#include "form_textureset.h"
#include "../../../dovah/forms/TextureSet.h"
#include "../asset_manager.h"

DovahKitAssetDataTextureSet::DovahKitAssetDataTextureSet(DovahKitAsset& o) : DovahKitAssetData(o) {
   for (auto& t : this->textures) {
      QObject::connect(&t, &DovahKitAssetReceptor::ready,  &this->owner, [this]() { this->dependencyResolved(); });
      QObject::connect(&t, &DovahKitAssetReceptor::failed, &this->owner, [this]() { this->dependencyResolved(); });
   }
}

bool DovahKitAssetDataTextureSet::load(dovah::form_stub& stub) {
   if (stub.formType != dovah::form_type::texture_set)
      return false;
   this->loaded = stub.load().ptr_cast<dovah::loaded_forms::TextureSet>();
   //
   this->pending_count = 0;
   for (size_t i = 0; i < 8; ++i) {
      auto* s = this->loaded->texture_by_index(i);
      if (!s)
         break;
      if (!s->empty())
         ++this->pending_count;
   }
   if (this->pending_count > 0) {
      auto& am = DovahKitAssetManager::get();
      for (size_t i = 0; i < 8; ++i) {
         auto* s = this->loaded->texture_by_index(i);
         if (!s)
            break;
         if (s->empty())
            continue;
         auto path = QLatin1Literal("textures/") + QString::fromStdString(*s);
         this->textures[i] = am.requestAsset(path);
      }
   }
   //
   return true;
}

void DovahKitAssetDataTextureSet::abandonDependencies() {
   for (auto& t : this->textures)
      t = nullptr;
   this->pending_count = 0;
}