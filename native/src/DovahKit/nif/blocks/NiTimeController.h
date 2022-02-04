#pragma once
#include "NiObject.h"

namespace nifDK::block_types {
   class NiObjectNET;

   class NiTimeController : public NiObject {
      public:
         static constexpr const char* const type_name = "NiTimeController";
      public:
         enum class animation_type {
            app_time,
            app_init,
         };
         enum class cycle_type {
            loop,
            reverse,
            clamp,
         };

         NiTimeController* next = nullptr;
         struct {
            animation_type anim_type  = animation_type::app_time;
            cycle_type     cycle_type = cycle_type::loop;
            bool active                = false;
            bool play_backwards        = false;
            bool is_manager_controlled = false;
            bool unknown               = true; // always set in Skyrim and Fallout; function unknown
         } config;
         float frequency  = 1.0;
         float phase      = 0.0;
         float start_time = FLT_MAX;
         float end_time   = -FLT_MAX;
         NiObjectNET* target = nullptr;
         struct {
            uint32_t     unknown = 0;
         } legacy_data;

         virtual void parse(file_reader&) override;
   };
}