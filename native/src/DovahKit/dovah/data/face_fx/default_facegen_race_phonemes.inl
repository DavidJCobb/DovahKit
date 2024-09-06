#pragma once
#include "./default_facegen_race_phonemes.h"

namespace dovah::face_fx {
   #pragma region Accessor functions
      constexpr const default_facegen_race_phonemes_type::weight_list_type& default_facegen_race_phonemes_type::operator[](phoneme p) const { return this->weights_by_phoneme[(size_t)p]; }
      constexpr default_facegen_race_phonemes_type::weight_list_type& default_facegen_race_phonemes_type::operator[](phoneme p) { return const_cast<weight_list_type&>(std::as_const(*this).operator[](p)); }

      constexpr const float& default_facegen_race_phonemes_type::weight(phoneme p, const std::string_view name) const {
         auto& list = this->weights_by_phoneme[(size_t)p];
         for (size_t i = 0; i < this->morph_names.size(); ++i)
            if (this->morph_names[i] == name)
               return list[i];
         throw std::out_of_range("bad morph name");
      }
      constexpr float& default_facegen_race_phonemes_type::weight(phoneme p, const std::string_view name) {
         return const_cast<float&>(std::as_const(*this).weight(p, name));
      }
   #pragma endregion
}