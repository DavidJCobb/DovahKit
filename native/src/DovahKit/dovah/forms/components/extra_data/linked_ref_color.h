#pragma once
#include "../extra_data.h"
#include "../../structs/color_dword.h"

namespace dovah::loaded_forms::components::extra {
   class linked_ref_color : public basic_extra_data {
      //
      // Not loaded by the game. Most likely intended for use exclusively within the 
      // Creation Kit.
      //
      public:
         static constexpr uint32_t signature = 'XCLP';
         //
         color_t start;
         color_t end;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::linked_ref_color; };
         virtual load_result load(tes_subrecord_reader&) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub*) {}
   };
}