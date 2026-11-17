#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "lu/enums/enum_flags.h"
#include "dovah/form_data/helpers/type_aliases.h"
#include "dovah/form_data/params.h"
#include "dovah/form_data/pieces/precached_nif_info.h"
#include "dovah/types/managed_form_use.h"
#include "dovah/types/managed_vector.h"
namespace dovah {
   namespace load_order_interfaces {
      class form_load;
   }
   namespace load_order_processes::use_info_build {
      class use_info_builder;
   }
   namespace tes {
      class subrecord_view;
   }

   namespace form_data::serialization {
      template<template<form_data_params> typename>
      void build_outbound_use_info(tes::subrecord_view&, load_order_processes::use_info_build::use_info_builder&);
   }
}

namespace dovah::form_data::components {
   // TESModel
   // Specifies a 3D model for a form.
   class model {
      protected:
         model(bool sts) : _supports_texture_swaps(sts) {}
      public:
         model() : _supports_texture_swaps(true) {}

         enum class facegen_flag {
            head,
            torso,
            hand_right,
            hand_left,
         };
         
      public:
         std::string                     model_path;     // MODL
         pieces::precached_nif_info      precached_info; // MODT
         lu::enum_flags<facegen_flag, 8> facegen_flags;  // MODD
      private:
         bool _supports_texture_swaps = false;

      public:
         constexpr bool has_precached_info() const noexcept { return !this->precached_info.empty(); }
         constexpr bool supports_texture_swaps() const noexcept { return this->_supports_texture_swaps; }

      public:
         void load_model_path(tes::subrecord_view&, load_order_interfaces::form_load&);
         void load_precached_info(tes::subrecord_view&, load_order_interfaces::form_load&);
         void load_facegen_flags(tes::subrecord_view&, load_order_interfaces::form_load&);
   };

   // TESModelTextureSwap
   // A 3D model that supports texture swaps for any NIF blocks with the appropriate shader flag.
   template<form_data_params Params>
   class model_ts : public model {
      public:
         #include "dovah/form_data/fragments/pull_type_aliases.inl"
         model_ts() : model(true) {}

         struct texture_swap {
            std::string nif_block_name;
            form_use    texture_set;    // -> TXST
            uint32_t    nif_leaf_index;
         
            #define CLASSNAME texture_swap
            #define TYPENAME  model_ts<FORM_DATA_PARAMS>::texture_swap
            #define PER_FIELD(X) \
               X(nif_block_name) \
               X(texture_set) \
               X(nif_leaf_index)
            #include "dovah/form_data/fragments/define_visitors.inl"
         };

      public:
         vector<texture_swap> texture_swaps;

         #define CLASSNAME model_ts
         #define TYPENAME  model_ts<FORM_DATA_PARAMS>
         #define PER_FIELD(X) \
            X(model_path) \
            X(precached_info) \
            X(facegen_flags) \
            X(texture_swaps)
         #include "dovah/form_data/fragments/define_visitors.inl"
   };
}

namespace dovah::form_data::serialization {
   extern bool load(components::model& self, tes::subrecord_view& subrecord, load_order_interfaces::form_load&);

   template<>
   extern void build_outbound_use_info<components::model_ts>(tes::subrecord_view&, load_order_processes::use_info_build::use_info_builder&);
   //
   extern void load(components::model_ts<managed_form_params>&, tes::subrecord_view&, load_order_interfaces::form_load&);
}