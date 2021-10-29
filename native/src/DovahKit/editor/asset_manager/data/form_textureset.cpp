#include "form_textureset.h"
#include "../../../dovah/forms/TextureSet.h"
#include "../asset_manager.h"

bool DovahKitAssetDataTextureSet::load(dovah::form_stub& stub) {
   if (stub.formType != dovah::form_type::texture_set)
      return false;
   this->loaded = stub.load().ptr_cast<dovah::loaded_forms::TextureSet>();
   //
   auto* p = this->pending;
   if (!p)
      p = this->pending = new path_list;
   //
   p->paths[0] = QString::fromStdString(this->loaded->texture_by_index<0>());
   p->paths[1] = QString::fromStdString(this->loaded->texture_by_index<1>());
   p->paths[2] = QString::fromStdString(this->loaded->texture_by_index<2>());
   p->paths[3] = QString::fromStdString(this->loaded->texture_by_index<3>());
   p->paths[4] = QString::fromStdString(this->loaded->texture_by_index<4>());
   p->paths[5] = QString::fromStdString(this->loaded->texture_by_index<5>());
   p->paths[6] = QString::fromStdString(this->loaded->texture_by_index<6>());
   p->paths[7] = QString::fromStdString(this->loaded->texture_by_index<7>());
   for(auto& path : p->paths)
      if (!path.isEmpty())
         path = QLatin1Literal("textures/") + path;
   //
   return true;
}

bool DovahKitAssetDataTextureSet::hasPendingDependencies() const {
   if (this->pending) {
      for (const auto& p : this->pending->paths)
         if (!p.isEmpty())
            return true;
   }
   return false;
}
void DovahKitAssetDataTextureSet::requestDependencies() {
   this->_requestDependencies(this->pending, this->textures);
}