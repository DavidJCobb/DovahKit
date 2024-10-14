#include "./nif_is_valid_master_particle_system.h"
#include "nif/file.h"
#include "nif/blocks/BSMasterParticleSystem.h"
#include "nif/blocks/BSPSysMultiTargetEmitterCtlr.h"
#include "nif/blocks/NiParticles.h"
#include "nif/blocks/NiParticleSystem.h"
#include "nif/blocks/NiPSysModifier.h"
#include "nif/blocks/NiPSysVolumeEmitter.h"

namespace {
   const nifDK::block_types::NiPSysModifier* _modifier_by_name(const nifDK::file& file, const std::string& name) {
      for (const auto* block : file.all_blocks) {
         if (auto* casted = dynamic_cast<const nifDK::block_types::NiPSysModifier*>(block)) {
            if (casted->name == name)
               return casted;
         }
      }
      return nullptr;
   }
}

namespace editor_helpers {
   extern bool nif_is_valid_master_particle_system(const nifDK::file& file) {
      auto* mps = dynamic_cast<const nifDK::block_types::BSMasterParticleSystem*>(file.root_node);
      if (!mps)
         return false;
      for (auto* block : mps->particle_systems) {
         if (!block)
            continue;
         auto* ps = dynamic_cast<const nifDK::block_types::NiParticleSystem*>(block);
         if (!ps || !ps->is_world_space)
            return false;
         bool valid = false;
         ps->for_each_controller([&valid, &file, mps](const nifDK::block_types::NiTimeController* ctrl) -> bool {
            auto* casted = dynamic_cast<const nifDK::block_types::BSPSysMultiTargetEmitterCtlr*>(ctrl);
            if (!casted)
               return false;
            if (casted->master_particle_system != mps)
               return true;

            auto* block = _modifier_by_name(file, casted->modifier_name);
            if (!block)
               return true;
            if (!dynamic_cast<const nifDK::block_types::NiPSysVolumeEmitter*>(block))
               return true;
            valid = true;
            return true;
         });
         if (!valid)
            return false;
      }
      return true;
   }
}