#pragma once
#include <string>
#include <vector>
#include "types.h"

class TESPluginSubrecord;
class FormStub;

struct FormModelTextureHash { // MODT
   std::vector<uint8_t> data; // bytes
};
struct FormModelTextureSwap { // MODS
   std::string nifBlockName;
   form_id_t   textureSet;
   uint32_t    nifBlockIndex;
};
struct FormModel { // MODL
   std::string modelPath;
   FormModelTextureHash textureHashes;
   std::vector<FormModelTextureSwap> textureSwaps;
   //
   void load(TESPluginSubrecord&);
   static void generateUseInfo(TESPluginSubrecord&, FormStub*);
};