#pragma once
#include <cstdint>
#include <utility> // std::hash
namespace dovah {
   namespace tes_file_reading {
      class subrecord;
   }
   namespace tes_file_writing {
      class subrecord;
   }
}

namespace dovah {
   struct form_id_t {
      //
      // This struct exists in order to allow the "read"/"write" functions for file I/O to be 
      // templated on form IDs, to automate form ID fixup.
      //
      // Loaded forms should not use this struct as a member. It should only be used for 
      // generating use info, i.e. when you need to load a form ID but not retain it.
      //
      friend class tes_file_reading::subrecord;
      friend class tes_file_writing::subrecord;
      template<typename T> friend struct ::std::hash;
      protected:
         uint32_t value = 0;

      public:
         constexpr form_id_t() {};
         constexpr form_id_t(uint32_t i) : value(i) {};
         
         constexpr operator uint32_t() const noexcept { return this->value; };
         
         constexpr bool operator>(const form_id_t& other) { return this->value > other.value; };
         constexpr bool operator<(const form_id_t& other) { return this->value < other.value; };
         constexpr bool operator>=(const form_id_t& other) { return this->value >= other.value; };
         constexpr bool operator<=(const form_id_t& other) { return this->value <= other.value; };
         constexpr bool operator==(const form_id_t& other) { return this->value == other.value; };
         constexpr bool operator!=(const form_id_t& other) { return this->value != other.value; };
   };
}

// Allow storage of form_id_t in std::set and friends:
namespace std {
   template<> struct hash<dovah::form_id_t> {
      size_t operator()(const dovah::form_id_t v) const {
         return std::hash<uint32_t>{}(v.value);
      }
   };
}