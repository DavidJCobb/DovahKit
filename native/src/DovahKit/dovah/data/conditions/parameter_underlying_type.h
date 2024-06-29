#pragma once

namespace dovah::conditions {
   enum class parameter_underlying_type {
      none,
      alias,        // ID of any alias on the condition's owning quest
      character,    // single-byte character, e.g. a 3D axis 'X' 'Y' or 'Z'
      enumeration,
      float32,
      form,
      int_signed,
      int_unsigned,
      package_data, // integer; index of a Package Data in the package containing the condition
      quest_stage,  // integer; allowed values depend on what QUST is in the previous argument
      string,       // string stored in the CIS1 and CIS2 subrecords
   };

   namespace c_types {
      using alias        = uint32_t;
      using event        = uint32_t;
      using int_signed   = int32_t;
      using int_unsigned = uint32_t;
      using package_data = uint32_t;
      using quest_stage  = uint32_t;
   }

   using enumeration_parameter_value = int32_t;
}