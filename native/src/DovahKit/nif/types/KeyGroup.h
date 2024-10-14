#pragma once
#include <cstdint>
#include "Key.h"
#include "KeyType.h"
#include "../reader.h"

namespace nifDK {
   class file_reader;

   template<typename ValueType>
   struct KeyGroup {
      using value_type = ValueType;

      KeyType interpolation_type = KeyType::linear;
      std::vector<Key<value_type>> keys;

      void read(file_reader& reader) {
         uint32_t count;
         reader.read(count);
         reader.read(this->interpolation_type);
         this->keys.resize(count);
         for (auto& key : this->keys) {
            key.read(reader, this->interpolation_type);
         }
      }
   };
}