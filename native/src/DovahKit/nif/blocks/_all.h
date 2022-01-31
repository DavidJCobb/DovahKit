#pragma once
#include <concepts>
#include <string>
#include "helpers/class_list.h"
#pragma region block types
   #include "unknown.h"
   //
#include "NiObject.h"
   #include "NiObjectNET.h"
      #include "NiAVObject.h"
         #include "NiGeometry.h"
            #include "NiTriBasedGeom.h"
               #include "NiTriShape.h"
         #include "NiNode.h"
#pragma endregion

namespace nifDK {
   using all_block_types = cobb::class_list<
      block_types::unknown_block,
      //
      block_types::NiAVObject,
      block_types::NiGeometry,
      block_types::NiNode,
      block_types::NiObject,
      block_types::NiObjectNET,
      block_types::NiTriBasedGeom,
      block_types::NiTriShape//,
   >;

   template<typename T> concept block_type_has_name = requires {
      { T::type_name } -> std::same_as<std::string>;
   };

}
