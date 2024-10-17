#pragma once
#include "../../../base_form_load_warning.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_load_warnings::by_type::scene {
   //
   // When the scene loader encounters ANAM with a valid value, it feeds all 
   // subsequent subrecords to the to-be-loaded scene action until it either 
   // finishes reading the action or encounters an irrecoverable problem. If 
   // an action is considered invalid, then the faulting subrecord will be 
   // skipped, all subsequent subrecords will be fed to the scene again, and 
   // the invalid action will not be retained in memory.
   // 
   // Scene actions are implemented as the BGSSceneAction base class, which 
   // has subclasses for each scene action type. BGSSceneAction will cause 
   // the loading of a scene action to fail in the following situations:
   // 
   //  - Any of ALID, INAM, SNAM, or ENAM fail to read (e.g. subrecord is 
   //    too short).
   // 
   //  - Any non-BGSSceneAction subrecord is encountered before ENAM is 
   //    encountered. (Valid subrecords are NAM0, ALID, INAM, FNAM, SNAM, 
   //    and ENAM.)
   //
   class scene_action_base_layout_incorrect : public base_form_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;
      public:
         enum class problem_type {
            malformed_valid_subrecord,
            unrecognized_subrecord,
         };

      public:
         constexpr scene_action_base_layout_incorrect(
            form_stub&   stub,
            uint32_t     action_id,
            uint32_t     subrecord_signature,
            problem_type problem
         )
         :
            base_form_load_warning(stub),
            action_id(action_id),
            subrecord_signature(subrecord_signature),
            problem(problem)
         {}

         uint32_t action_id;
         uint32_t subrecord_signature;
         problem_type problem;
   };
}
#include "../../../_util.undef.h"