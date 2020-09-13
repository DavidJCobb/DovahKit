#pragma once
#include <string>
#include <vector>
#include "../_common.h"

namespace dovah::loaded_forms::components {
   struct model_texture_hash { // MODT
      std::vector<uint8_t> data; // bytes
      //
      // Skyrim still loads this data, but we don't necessarily know how. We'd need 
      // to reverse-engineer {void LoadMODTSubrecord(TESModel*, BGSLoadFormBuffer*) 
      // at 0x00454AF0 in Skyrim Classic to learn more. I do know for certain, how-
      // ever, that the loading behavior changes depending on the form version.
      //
      // xEdit has this decoded.
      //
   };
   struct model_texture_swap { // MODS
      std::string nifBlockName;
      form_id_t   textureSet;
      uint32_t    nifBlockIndex;
   };
   struct model { // MODL
      std::string modelPath;
      model_texture_hash textureHashes;
      std::vector<model_texture_swap> textureSwaps;
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
   };
}