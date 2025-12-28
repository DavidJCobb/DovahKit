#pragma once
#include "NiProperty.h"

namespace nifDK::block_types {
   class NiAlphaProperty : public NiProperty {
      public:
         static constexpr const char* const type_name = "NiAlphaProperty";
      public:
         enum class blend_mode {
            one,
            zero,
            source_color,
            one_minus_source_color,
            destination_color,
            one_minus_destination_color,
            source_alpha,
            one_minus_source_alpha,
            destination_alpha,
            one_minus_destination_alpha,
            source_alpha_saturate,
         };
         enum class test_mode {
            always,
            less,
            equal,
            less_or_equal,
            greater,
            not_equal,
            greater_or_equal,
            never,
         };

         struct {
            bool       enabled     = false;
            blend_mode source      = blend_mode::source_color;
            blend_mode destination = blend_mode::one_minus_source_color;
         } blending;
         struct {
            bool      enabled      = false;
            bool      clone_unique = false;
            bool      configurable = false; // control whether a ref's ExtraAlphaCutoff can override the threshold
            test_mode mode         = test_mode::less;
            uint8_t   threshold    = 128;
            bool      no_sorter    = false;
         } testing;

         virtual void parse(file_reader&) override;
   };
}