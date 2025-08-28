#pragma once
#include <string>
#include <vector>
#include "dovah/forms/structs/precached_nif_info.h"
#include "./nif_texture_swap.h"

namespace dovah {
   namespace loaded_forms {
      namespace components {
         class model;
      }
      class Form;
   }
}

namespace ui::types {
   //
   // Represents a NIF file (and, potentially, texture swap options) for a form. The in-engine 
   // classnames for this would be TESModel and TESModelTextureSwap.
   //
   struct nif_for_form {
      std::string model_path;
      dovah::loaded_forms::precached_nif_info precached_nif_info;
      //
      bool supports_texture_swaps = false;
      std::vector<ui::types::nif_texture_swap> texture_swaps;

      void initializeFrom(const dovah::loaded_forms::components::model&);
      void commitTo(dovah::loaded_forms::components::model&, dovah::loaded_forms::Form& owner) const;
   };
}