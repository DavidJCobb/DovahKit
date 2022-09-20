#pragma once
#include <glm/glm.hpp>
#include "NiObject.h"

namespace nifDK::block_types {
   class BSMultiBoundData;

   class BSMultiBound : public NiObject {
      public:
         static constexpr const char* const type_name = "BSMultiBound";
      public:
         BSMultiBoundData* data = nullptr; // unowned

         virtual void parse(file_reader&) override;
   };

   class BSMultiBoundData : public NiObject {
      public:
         static constexpr const char* const type_name = "BSMultiBoundData";
   };

   class BSMultiBoundDataOBB : public BSMultiBoundData {
      public:
         static constexpr const char* const type_name = "BSMultiBoundDataOBB";
      public:
         glm::vec3  center;
         glm::vec3  sizes;
         NiMatrix33 rotation;
         
         virtual void parse(file_reader&) override;
   };

   class BSMultiBoundDataSphere : public BSMultiBoundData {
      public:
         static constexpr const char* const type_name = "BSMultiBoundDataSphere";
      public:
         glm::vec3 center;
         float     radius;
         
         virtual void parse(file_reader&) override;
   };
}