#pragma once
#include <cstdint>
#include "lu/integral_character_code.h"
#include "dovah/form_data/helpers/type_aliases.h"
#include "dovah/form_data/params.h"
#include "dovah/types/form_id.h"
#include "dovah/types/managed_form_use.h"
namespace dovah {
   namespace form_data::serialization {
      template<template<form_data_params> typename>
      struct pending_uses;
   }
   namespace load_order_interfaces {
      class form_load;
   }
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   namespace tes {
      class subrecord_view;
   }
}

namespace dovah::form_data::components {
   // TESEnchantableForm
   template<form_data_params Params>
   class enchantable {
      public:
         #include "dovah/form_data/fragments/pull_type_aliases.inl"

         static constexpr const lu::four_cc subrecord_signature_effect        = 'EITM'; // Effect ITeM
         static constexpr const lu::four_cc subrecord_signature_charge        = 'EAMT'; // Enchantment AMounT
         static constexpr const lu::four_cc subrecord_signature_effect_legacy = 'ENAM'; // changed to EITM since Fallout 3
         static constexpr const lu::four_cc subrecord_signature_charge_legacy = 'ANAM'; // changed to EAMT since Fallout 3

      public:
         form_use effect; // -> ENCH|SPEL
         uint16_t charge = 0;

         #define CLASSNAME enchantable
         #define TYPENAME  enchantable<FORM_DATA_PARAMS>
         #define PER_FIELD(X) \
            X(effect) \
            X(charge)
         #include "dovah/form_data/fragments/define_visitors.inl"
   };
}

namespace dovah::form_data::serialization {
   template<>
   struct pending_uses<components::enchantable> {
      form_id effect;

      void read(tes::subrecord_view&);
      void commit_to(load_order_processes::use_info_build::use_info_builder&);
   };

   extern void load(components::enchantable<managed_form_params>&, tes::subrecord_view&, load_order_interfaces::form_load&);
}