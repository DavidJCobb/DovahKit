#pragma once
#include <type_traits>
#include "NiAVObject.h"

namespace nifDK::block_types {
   class NiDynamicEffect;

   class NiNode : public NiAVObject {
      public:
         static constexpr const char* const type_name = "NiNode";
      public:
         std::vector<NiAVObject*>      children;
         std::vector<NiDynamicEffect*> effects;

         virtual void parse(file_reader&) override;

         template<typename T> requires std::is_base_of_v<NiAVObject, T>
         size_t count_descendants_of_type() const {
            size_t count = 0;
            for (auto* child : this->children) {
               auto* node = dynamic_cast<NiNode*>(child);
               if constexpr (std::is_base_of_v<NiNode, T>) {
                  //
                  // If T derives from NiNode, then we can skip the cast to T if the prior 
                  // cast to NiNode (which we need for recursion) failed.
                  //
                  if (!node)
                     continue;
                  if (auto* casted = dynamic_cast<T*>(child)) {
                     ++count;
                  }
               } else {
                  if (auto* casted = dynamic_cast<T*>(child)) {
                     ++count;
                  }
                  if (!node)
                     continue;
               }
               count += node->count_descendants_of_type<T>();
            }
            return count;
         }
         template<typename T> requires (!std::is_base_of_v<NiAVObject, T>)
         inline size_t count_descendants_of_type() const {
            //
            // The "children" vector can only store NiAVObjects; a NiNode by definition cannot have 
            // any descendants of a type that doesn't subclass NiAVObject.
            //
            return 0;
         }

         template<typename T> void for_self_and_subtree(T lambda) {
            (lambda)(this);
            for (auto* child : this->children) {
               auto* node = dynamic_cast<NiNode*>(child);
               if (!node)
                  continue;
               node->for_self_and_subtree(lambda);
            }
         }
   };
}