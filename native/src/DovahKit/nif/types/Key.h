#pragma once
#include <cstdint>
#include <optional>
#include "KeyType.h"
#include "../reader.h"

namespace nifDK {
   class file_reader;

   template<typename ValueType>
   struct Key {
      using value_type = ValueType;

      struct quadratic_interpolation_parameters {
         value_type forward;
         value_type backward;
      };

      float      time     = 0;
      value_type value    = {};
      struct {
         //
         // Whether these are present depends on the KeyType.
         //
         std::optional<quadratic_interpolation_parameters> quadratic;
         std::optional<value_type> tension_bias_continuity;
      } interpolation;

      void read(file_reader& reader, KeyType type) {
         reader.read(this->time);
         reader.read(this->value);
         switch (type) {
            case KeyType::quadratic:
               {
                  auto& dst = this->interpolation.quadratic.emplace();
                  reader.read(dst.forward);
                  reader.read(dst.backward);
               }
               break;
            case KeyType::tension_bias_continuity:
               reader.read(this->interpolation.tension_bias_continuity.emplace());
               break;
         }
      }
   };
}