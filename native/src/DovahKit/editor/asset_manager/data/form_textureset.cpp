#include "form_textureset.h"
#include "../../../dovah/forms/TextureSet.h"
#include "../asset_manager.h"

namespace {
   static constexpr size_t TEXTURE_COUNT = 8;
}

DovahKitAssetDataTextureSet::DovahKitAssetDataTextureSet(DovahKitAsset& o) : DovahKitAssetData(o) {
   for (auto& t : this->textures) {
      QObject::connect(&t, &DovahKitAssetReceptor::ready,  &this->owner, [this]() { this->dependencyResolved(); });
      QObject::connect(&t, &DovahKitAssetReceptor::failed, &this->owner, [this]() { this->dependencyResolved(); });
   }
}

bool DovahKitAssetDataTextureSet::load(dovah::form_stub& stub) {
   if (stub.form_type != dovah::form_type::texture_set)
      return false;
   this->loaded = stub.load().ptr_cast<dovah::loaded_forms::TextureSet>();
   //
   auto& am = DovahKitAssetManager::get();
   for (size_t i = 0; i < TEXTURE_COUNT; ++i) {
      auto* s = this->loaded->texture_by_index(i);
      if (s->empty())
         continue;
      auto path = QLatin1Literal("textures/") + QString::fromStdString(*s);
      this->textures[i] = am.requestAsset(path);
   }
   //
   return true;
}

bool DovahKitAssetDataTextureSet::onFormModified() {
   if (!this->loaded)
      return false;
   //
   bool  changed = false;
   auto& am      = DovahKitAssetManager::get();
   for (size_t i = 0; i < TEXTURE_COUNT; ++i) {
      auto& receptor = this->textures[i];
      auto* s        = this->loaded->texture_by_index(i);
      assert(s);
      //
      QString path;
      if (!s->empty()) {
         path = DovahKitAssetManager::normalizeAssetPath(QLatin1Literal("textures/") + QString::fromStdString(*s));
      }
      if (receptor) {
         if (!receptor->samePathAs(path)) {
            changed  = true;
            receptor = am.requestAsset(path);
         }
      } else if (!path.isEmpty()) {
         changed  = true;
         receptor = nullptr;
      }
   }
   return changed;
}

bool DovahKitAssetDataTextureSet::hasPendingDependencies() const {
   for (const auto& t : this->textures)
      if (t != nullptr && !t.isReady() && !t.isFailed())
         return true;
   return false;
}
void DovahKitAssetDataTextureSet::abandonDependencies() {
   for (auto& t : this->textures)
      t = nullptr;
}

void DovahKitAssetDataTextureSet::dependencyResolved() {
   if (!this->hasPendingDependencies())
      emit this->owner.dependenciesLoaded();
}