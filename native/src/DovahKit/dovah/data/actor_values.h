#pragma once
#include <cstdint>

namespace dovah {
   namespace data {
      struct actor_value_info {
         uint32_t    index;
         uint32_t    formID;
         const char* name = "";
         //
         actor_value_info(uint32_t i, uint32_t f, const char* n) : index(i), formID(f), name(n) {};
         actor_value_info(uint32_t i, const char* n); // form ID can be computed from AV index
         //
         // TODO: store all of the same values as TESV.exe's ActorValueInfo class. 
         // Technically that class is a subclass of TESForm; we want to store that 
         // data so we can clone it into a LoadedForm::ActorValueInfo instance 
         // later. Look at ActorValueList::DefineActorValues (0x005AD830).
         //
      };
      struct actor_Value_info_list {
         actor_value_info* list = nullptr;
         uint32_t count;
         //
         actor_Value_info_list();
         ~actor_Value_info_list();
         //
         inline static actor_Value_info_list& get() {
            static actor_Value_info_list instance;
            return instance;
         }
      };
   }
}
