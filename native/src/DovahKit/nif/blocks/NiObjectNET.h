#pragma once
#include "NiObject.h"
#include "NiTimeController.h"

namespace nifDK::block_types {
   class NiExtraData;
   class NiTimeController;

   class NiObjectNET : public NiObject { // "NiObject with name, extra data, and time controller"
      public:
         static constexpr const char* const type_name = "NiObjectNET";
      public:
         std::string       name; // uint32_t length; chars;
         std::vector<NiExtraData*> extra;
         NiTimeController* controller = nullptr;

         template<typename Functor> requires std::is_invocable_v<Functor, NiTimeController*>
         constexpr void for_each_controller(Functor&& functor) {
            for (auto* block = this->controller; block; block = block->next) {
               if constexpr (std::is_invocable_r_v<bool, Functor, NiTimeController*>) {
                  if ((functor)(block))
                     break;
               } else {
                  (functor)(block);
               }
            }
         }

         template<typename Functor> requires std::is_invocable_v<Functor, const NiTimeController*>
         constexpr void for_each_controller(Functor&& functor) const {
            for (const auto* block = this->controller; block; block = block->next) {
               if constexpr (std::is_invocable_r_v<bool, Functor, NiTimeController*>) {
                  if ((functor)(block))
                     break;
               } else {
                  (functor)(block);
               }
            }
         }

         virtual void parse(file_reader&) override;
   };
}