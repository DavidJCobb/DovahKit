#pragma once
#include <vector>
#include "../types.h"

class TESPluginSubrecord;
class FormStub;

struct ContainerEntry {
   // CNTO:
   form_id_t item;
   int32_t   count = 0;
   // COED:
   form_id_t owner;
   union {
      form_id_t global;
      int32_t   factionRank = 0;
   };
   float     condition;
   //
   ContainerEntry() : factionRank(0) {};
};
struct ContainerData {
   std::vector<ContainerEntry> entries;
   //
   void load(TESPluginSubrecord&);
   static void generateUseInfo(TESPluginSubrecord&, FormStub*);
};