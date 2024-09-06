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
}

#include "./default_facegen_race_phonemes.inl"

namespace dovah::face_fx {
   constexpr const default_facegen_race_phonemes_type default_facegen_race_phonemes = []() -> default_facegen_race_phonemes_type {
      default_facegen_race_phonemes_type out;

      out.morph_names = {{
         "Aah", "BigAah", "BMP",  "ChJSh", "DST",
         "Eee", "Eh",     "FV",   "I",     "K",
         "N",   "Oh",     "OohQ", "R",     "Th",
         "W"
      }};

      out.weight(phoneme::IY, "Eee") = 1.0F;

      out.weight(phoneme::IH, "Eee") = 0.2F;
      out.weight(phoneme::IH, "Eh")  = 0.7F;

      out.weight(phoneme::EH, "Eh") = 1.0F;

      out.weight(phoneme::EY, "Aah") = 0.6F;
      out.weight(phoneme::EY, "Eee") = 0.3F;

      out.weight(phoneme::AE, "Aah") = 0.8F;

      out.weight(phoneme::AA, "Aah") = 1.0F;

      out.weight(phoneme::AW, "Aah") = 0.2F;
      out.weight(phoneme::AW, "Oh")  = 0.6F;

      out.weight(phoneme::AY, "Eee") = 0.2F;
      out.weight(phoneme::AY, "I")   = 0.8F;

      out.weight(phoneme::AH, "Aah") = 0.8F;
      out.weight(phoneme::AH, "Oh")  = 0.2F;

      out.weight(phoneme::AO, "Aah") = 0.1F;
      out.weight(phoneme::AO, "Oh")  = 0.5F;
      out.weight(phoneme::AO, "W")   = 0.2F;

      out.weight(phoneme::OY, "Eee") = 0.3F;
      out.weight(phoneme::OY, "Oh")  = 0.6F;

      out.weight(phoneme::OW, "BigAah") = 0.8F;
      out.weight(phoneme::OW, "Oh") = 0.2F;

      out.weight(phoneme::UH, "BigAah") = 0.5F;
      out.weight(phoneme::UH, "Oh") = 0.2F;

      out.weight(phoneme::UW, "OohQ") = 1.0F;

      out.weight(phoneme::ER, "I") = 0.5F;
      out.weight(phoneme::ER, "R") = 0.3F;

      out.weight(phoneme::AX, "BigAah") = 0.8F;

      out.weight(phoneme::S, "DST") = 0.9F;

      out.weight(phoneme::SH, "ChJSh") = 0.8F;

      out.weight(phoneme::Z, "DST") = 0.8F;

      out.weight(phoneme::ZH, "ChJSh") = 0.7F;

      out.weight(phoneme::F, "FV") = 0.8F;

      out.weight(phoneme::TH, "Th") = 1.0F;

      out.weight(phoneme::V, "FV") = 1.0F;

      out.weight(phoneme::DH, "DST") = 0.2F;
      out.weight(phoneme::DH, "Th")  = 0.8F;

      out.weight(phoneme::M, "BMP") = 1.0F;

      out.weight(phoneme::N, "N") = 1.0F;

      out.weight(phoneme::NG, "N") = 0.9F;

      out.weight(phoneme::L, "N")  = 0.6F;
      out.weight(phoneme::L, "Th") = 0.2F;

      out.weight(phoneme::R, "R") = 1.0F;

      out.weight(phoneme::W, "W") = 1.0F;

      out.weight(phoneme::Y, "Aah") = 0.2F;
      out.weight(phoneme::Y, "Eee") = 0.4F;
      out.weight(phoneme::Y, "I")   = 0.4F;

      out.weight(phoneme::HH, "Aah") = 0.6F;

      out.weight(phoneme::B, "BMP") = 1.0F;

      out.weight(phoneme::D, "DST") = 1.0F;

      out.weight(phoneme::JH, "ChJSh") = 1.0F;

      out.weight(phoneme::G, "Aah")   = 0.4F;
      out.weight(phoneme::G, "ChJSh") = 0.3F;

      out.weight(phoneme::P, "BMP") = 0.9F;

      out.weight(phoneme::T, "DST") = 0.8F;

      out.weight(phoneme::K, "K") = 1.0F;

      out.weight(phoneme::CH, "ChJSh") = 1.0F;

      // SIL is all zeroes.
      // SHORTSIL is all zeroes.
      // (Do these stand for "silence" and "short silence?")

      // FLAP is all zeroes.

      return out;
   }();
}