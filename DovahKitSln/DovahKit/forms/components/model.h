#pragma once
#include <string>
#include <vector>
#include "../types.h"

class TESPluginSubrecord;
class FormStub;

struct FormModelTextureHash { // MODT
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