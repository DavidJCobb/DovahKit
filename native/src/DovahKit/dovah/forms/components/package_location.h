#pragma once
#include "../_common.h"

namespace dovah::loaded_forms::components {
   enum class package_location_type : uint32_t {
      near_reference              =  0,
      in_cell                     =  1,
      near_package_start_location =  2,
      near_editor_location        =  3,
      object_id                   =  4,
      object_type                 =  5,
      near_linked_reference       =  6,
      at_package_location         =  7,
      reference_alias             =  8,
      location_alias              =  9,
      unknown_10                  = 10,
      unknown_11                  = 11,
      near_self                   = 12,
   };

   struct package_location {
      package_location_type type = package_location_type::near_reference;
      struct {
         //
         // This would be a union, except that the default constructor for (form_id_t) is "non-trivial" despite 
         // doing literally nothing. Only implicit constructors can be "trivial" (even foo() = default doesn't 
         // work), and those only get generated if a struct has *no* constructors. Ugh.
         //
         form_id_t form;
         uint32_t  object_type = 0;
         int32_t   alias_id    = 0;
         uint32_t  padding     = 0;
      } detail;
      int32_t radius = 0;
      //
      void load(tes_subrecord_reader&);
      static void generateUseInfo(tes_subrecord_reader&, form_stub*);
      void save(tes_subrecord_writer&);
      void clone_from(const package_location& original, form_stub& owner_of_clone) noexcept;
      void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
   };
}