#pragma once
#include "./base.h"

namespace dovah::use_info::entry_flags {
   enum class base_extra_data : underlying_type {
      extra_encounter_zone = first_form_type_specific_flag, // ****/XEZN
      extra_location,             // ****/XLCN
      extra_location_ref_type,    // ****/XLRT
      extra_teleport_destination, // ****/XTEL+0x00

      __COUNT
   };
   constexpr const size_t first_non_extra_data_flag = (size_t)base_extra_data::__COUNT;
}

namespace dovah::use_info {
   template<>
   struct is_entry_flag_type<entry_flags::base_extra_data> {
      static constexpr const bool value = true;
   };


   template<>
   struct entry_flag_type_form_types<entry_flags::base_extra_data> {
      static constexpr const auto value = std::array{ form_type::cell, form_type::reference };
   };
}