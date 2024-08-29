#pragma once
#include <array>
#include <stdexcept>
#include <string_view>
#include "./phonemes.h"

namespace dovah::face_fx {
   struct default_facegen_race_phonemes_type {
      public:
         static constexpr const size_t weight_count = 16;

         union weight_list_type {
            std::array<float, weight_count> all = {};
            struct {
               float Aah;
               float BigAah;
               float BMP;
               float ChJSh;
               float DST;
               float Eee;
               float Eh;
               float FV;
               float I;
               float K;
               float N;
               float Oh;
               float OohQ;
               float R;
               float Th;
               float W;
            };

            constexpr const float& operator[](size_t n) const { return this->all[n]; }
            constexpr float& operator[](size_t n) { return const_cast<float&>(std::as_const(*this).operator[](n)); }
         };

      public:
         std::array<std::string_view, weight_count>  morph_names;
         std::array<weight_list_type, phoneme_count> weights_by_phoneme = {};

         constexpr const weight_list_type& operator[](phoneme p) const;
         constexpr weight_list_type& operator[](phoneme p);

         //
         // Convenience accessors, for creating the sole constexpr instance of this struct. 
         // These throw on access if the name is wrong, which is fine for constexpr, but 
         // for run-time you should access the named fields in the union defined above.
         //
         constexpr const float& weight(phoneme p, const std::string_view name) const;
         constexpr float& weight(phoneme p, const std::string_view name);
   };

   constexpr const default_facegen_race_phonemes_type default_facegen_race_phonemes;
}

#include "./default_facegen_race_phonemes.inl"