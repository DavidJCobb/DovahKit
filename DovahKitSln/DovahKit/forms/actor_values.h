#pragma once
#include <cstdint>

struct ActorValueInfo {
   uint32_t    index;
   uint32_t    formID;
   const char* name = "";
   //
   ActorValueInfo(uint32_t i, uint32_t f, const char* n) : index(i), formID(f), name(n) {};
   ActorValueInfo(uint32_t i, const char* n); // form ID can be computed from AV index
   //
   // TODO: store all of the same values as TESV.exe's ActorValueInfo class. 
   // Technically that class is a subclass of TESForm; we want to store that 
   // data so we can clone it into a LoadedForm::ActorValueInfo instance 
   // later. Look at ActorValueList::DefineActorValues (0x005AD830).
   //
};
struct ActorValueInfoList {
   ActorValueInfo* list = nullptr;
   uint32_t count;
   //
   ActorValueInfoList();
   ~ActorValueInfoList();
   //
   inline static ActorValueInfoList& get() {
      static ActorValueInfoList instance;
      return instance;
   }
};