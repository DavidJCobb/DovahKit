#pragma once
#include <array>
#include <string_view>
#include "./package_data_type.h"

namespace dovah::packages {
   enum class procedure_type {
      invalid = -1,

      acquire,           // "Acquire"
      activate,          // "Activate"
      dialogue_activate, // "DialogueActivate"
      dialogue,          // "Dialogue"
      done,              // "Done"
      eat,               // "Eat"
      escort,            // "Escort"
      find,              // "Find"
      flee,              // "Flee"
      flight_grab,       // "FlightGrab"
      follow,            // "Follow"
      follow_to,         // "FollowTo"
      force_greet,       // "ForceGreet"
      guard,             // "Guard"
      hold_position,     // "HoldPosition"
      hover,             // "Hover"
      keep_an_eye_on,    // "KeepAnEyeOn"
      lock_doors,        // "LockDoors"
      orbit,             // "Orbit"
      patrol,            // "Patrol"
      pursue,            // "Pursue"
      sandbox,           // "Sandbox"
      say,               // "Say"
      shout,             // "Shout"
      sit,               // "Sit"
      sleep,             // "Sleep"
      travel,            // "Travel"
      unlock_doors,      // "UnlockDoors"
      use_idle_marker,   // "UseIdleMarker"
      use_magic,         // "UseMagic"
      use_weapon,        // "UseWeapon"
      wait,              // "Wait"
      wander,            // "Wander"
   };

   struct procedure_type_info {
      public:
         enum class param_type {
            none,
            boolean,
            float32,
            integer,
            location,
            object_list,
            target, // IAITarget // "Target"
            target_selector,
            topic,
         };
         struct param_info {
            std::string_view name;
            bool             required = false;
            param_type       type     = param_type::none;

            // based on manual testing within the CK UI
            constexpr bool accepts(const package_data_type pdt) const noexcept {
               switch (this->type) {
                  case param_type::boolean: return pdt == package_data_type::boolean;
                  case param_type::float32: return pdt == package_data_type::float32;
                  case param_type::integer: return pdt == package_data_type::integer;
                  case param_type::location:
                     switch (pdt) {
                        case package_data_type::location:
                        case package_data_type::object_list:
                        case package_data_type::single_ref:
                           return true;
                     }
                     break;
                  case param_type::object_list: return pdt == package_data_type::object_list;
                  case param_type::target:
                     switch (pdt) {
                        case package_data_type::object_list:
                        case package_data_type::single_ref:
                           return true;
                     }
                     break;
                  case param_type::target_selector: return pdt == package_data_type::target_selector;
                  case param_type::topic:           return pdt == package_data_type::topic;
               }
               return false;
            }
         };

      public:
         procedure_type type;

         size_t param_count = 0;
         std::array<param_info, 19> params;
   };

   inline static constexpr const auto all_procedure_type_info = []() {
      using param_info = procedure_type_info::param_info;
      using param_type = procedure_type_info::param_type;
      return std::array{
         procedure_type_info{
            .type = procedure_type::acquire,
            .param_count = 5,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "NumToAcquire",
                  .required = false,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "AllowStealing",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowPickpocketing",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowKilling",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::activate,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "NumToActivate",
                  .required = true,
                  .type     = param_type::integer,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::dialogue_activate,
            .param_count = 3,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "LookAtTarget",
                  .required = true,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::dialogue,
            .param_count = 5,
            .params = {
               param_info{
                  .name     = "Topic",
                  .required = true,
                  .type     = param_type::topic,
               },
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "FinishSpeaking",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "LookAtTarget",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "ForceSubtitle",
                  .required = true,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::done,
            .param_count = 0,
         },
         procedure_type_info{
            .type = procedure_type::eat,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "FoodType",
                  .required = false,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "CreateFakeFood",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::escort,
            .param_count = 9,
            .params = {
               param_info{
                  .name     = "EscortedActor(s)",
                  .required = false,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "NumberToEscort",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "Destination",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "EscortWaitDist",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "FollowMinDist",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "FollowMaxDist",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "PreferPreferredPath",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RunIfBehindDist",
                  .required = false,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::find,
            .param_count = 5,
            .params = {
               param_info{
                  .name     = "SearchLocation",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "TargetSelector",
                  .required = true,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "ObjectList",
                  .required = false,
                  .type     = param_type::object_list,
               },
               param_info{
                  .name     = "SearchOwnInventory",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RandomizeList",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::flee,
            .param_count = 10,
            .params = {
               param_info{
                  .name     = "FleeFrom",
                  .required = false,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "FleeTo",
                  .required = false,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "UseDynamicGoalInstead",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "GoalRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "AvoidNodeRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "FleeDist",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "ForgetDeadThreats",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "Quiet",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "CountsAsIsFleeing",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::flight_grab,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "DisableCollisionChecks",
                  .required = true,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::follow,
            .param_count = 6,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "MinRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "MaxRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "GoToLeadersGoal",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "NeedLOS",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::follow_to,
            .param_count = 7,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "EndLocation",
                  .required = false,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "MinRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "MaxRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "GoToLeadersGoal",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "NeedLOS",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::force_greet,
            .param_count = 3,
            .params = {
               param_info{
                  .name     = "Topic",
                  .required = true,
                  .type     = param_type::topic,
               },
               param_info{
                  .name     = "NpcWaitLocation",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "TargetTriggerLocation",
                  .required = true,
                  .type     = param_type::location,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::guard,
            .param_count = 4,
            .params = {
               param_info{
                  .name     = "RestrictedArea",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "SuspiciousOf",
                  .required = false,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "WarnOnlyRadius",
                  .required = false,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "ImmediateAttackRadius",
                  .required = false,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::hold_position,
            .param_count = 1,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::hover,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "TargetLocation",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "Height",
                  .required = true,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::keep_an_eye_on,
            .param_count = 3,
            .params = {
               param_info{
                  .name     = "TargetToObserve",
                  .required = false,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "ObservationArea",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "EndPursuitArea",
                  .required = false,
                  .type     = param_type::location,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::lock_doors,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "WarnBeforeLocking",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::orbit,
            .param_count = 4,
            .params = {
               param_info{
                  .name     = "TargetLocation",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "InnerRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "OuterRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "Height",
                  .required = true,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::patrol,
            .param_count = 6,
            .params = {
               param_info{
                  .name     = "PathStart",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "PointRadius",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "Repeatable",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "StartAtNearestPoint",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "StaticPathing",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::pursue,
            .param_count = 4,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "WhileWithinLocation",
                  .required = false,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "UntilWithinLocation",
                  .required = false,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "WarnToLeave",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::sandbox,
            .param_count = 11,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "AllowEating",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowSleeping",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowConversation",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowIdleMarkers",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowSitting",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AllowWandering",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "WanderPreferredPath",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "Energy",
                  .required = false,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "AllowSpecialFurniture",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "MinWanderDistance",
                  .required = false,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::say,
            .param_count = 5,
            .params = {
               param_info{
                  .name     = "Topic",
                  .required = true,
                  .type     = param_type::topic,
               },
               param_info{
                  .name     = "Target",
                  .required = false,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "FinishSpeaking",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "LookAtTarget",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "ForceSubtitle",
                  .required = true,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::shout,
            .param_count = 5,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "Shout",
                  .required = true,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "HoldWhenBlocked",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "JustHeadtrack",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::sit,
            .param_count = 1,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::sleep,
            .param_count = 1,
            .params = {
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::travel,
            .param_count = 3,
            .params = {
               param_info{
                  .name     = "Destination",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "PreferPreferredPath",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::unlock_doors,
            .param_count = 1,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::use_idle_marker,
            .param_count = 1,
            .params = {
               param_info{
                  .name     = "Marker",
                  .required = true,
                  .type     = param_type::target,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::use_magic,
            .param_count = 11,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "Spell",
                  .required = true,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "HoldWhenBlocked",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "CastTimeMin",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "CastTimeMax",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "CooldownTimeMin",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "CooldownTimeMax",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "NumToCastMin",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "NumToCastMax",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "DualCast",
                  .required = false,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::use_weapon,
            .param_count = 19,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "Weapon",
                  .required = true,
                  .type     = param_type::target_selector,
               },
               param_info{
                  .name     = "Target",
                  .required = true,
                  .type     = param_type::target,
               },
               param_info{
                  .name     = "AlwaysHit",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RepeatFire",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "VolleyFire",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "CrouchReload",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "DoNoDamage",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "HoldWhenBlocked",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "VolleyWaitMin",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "VolleyWaitMax",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "VolleysPerBurst",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "VolleyShotsMin",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "VolleyShotsMax",
                  .required = true,
                  .type     = param_type::integer,
               },
               param_info{
                  .name     = "AllowCombatStart",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AimWithoutFiring",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "AlwaysPowerAttack",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "DoHeadtracking",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "BlockPercent",
                  .required = false,
                  .type     = param_type::float32,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::wait,
            .param_count = 2,
            .params = {
               param_info{
                  .name     = "ActualSeconds",
                  .required = true,
                  .type     = param_type::float32,
               },
               param_info{
                  .name     = "StopMovement",
                  .required = true,
                  .type     = param_type::boolean,
               },
            },
         },
         procedure_type_info{
            .type = procedure_type::wander,
            .param_count = 4,
            .params = {
               param_info{
                  .name     = "Location",
                  .required = true,
                  .type     = param_type::location,
               },
               param_info{
                  .name     = "Preferred Path Only",
                  .required = true,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "RideHorseIfPossible",
                  .required = false,
                  .type     = param_type::boolean,
               },
               param_info{
                  .name     = "MinimumDistance",
                  .required = false,
                  .type     = param_type::float32,
               },
            },
         },
      };
   }();

   static_assert(
      []() -> bool {
      for (size_t i = 0; i < all_procedure_type_info.size(); ++i)
         if ((size_t)all_procedure_type_info[i].type != i)
            return false;
         return true;
      }(),
      "Procedure type info must be contiguous and ordered."
   );
   static_assert(
      []() -> bool {
         using param_type = procedure_type_info::param_type;
         for (size_t i = 0; i < all_procedure_type_info.size(); ++i) {
            auto&  info  = all_procedure_type_info[i];
            size_t count = info.param_count;
            for (size_t i = 0; i < info.params.size(); ++i) {
               if (i < count) {
                  if (info.params[i].type == param_type::none)
                     return false;
               } else {
                  if (info.params[i].type != param_type::none)
                     return false;
               }
            }
         }
         return true;
      }(),
      "The param-count values must match the param lists for each procedure's info."
   );
}
