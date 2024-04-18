#pragma once
#include <cstdint>
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_component::destruction {
   //
   // Destruction data first declares the number of stages; then, each stage declares 
   // its own index within the array. In other words, stages can be serialized out of 
   // order, though in practice Bethesda never seems to do this.  Stages that specify 
   // an out-of-bounds index will be discarded.
   //
   class stage_serialized_index_out_of_bounds : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         constexpr stage_serialized_index_out_of_bounds(
            form_stub& subject,
            size_t which_subrecord,
            size_t serialized_stage_index,
            size_t stage_count
         )
         :
            base_form_load_warning(subject),
            which_subrecord(which_subrecord),
            serialized_stage_index(serialized_stage_index),
            stage_count(stage_count)
         {}

         size_t which_subrecord;
         size_t serialized_stage_index; // stage index (which will have been out of bounds)
         size_t stage_count; // size of the list
   };
}
#include "../../../_util.undef.h"