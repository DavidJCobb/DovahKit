#include "_use_info.h"
#include "../../../form_stub_use_info_builder.h"

namespace dovah {
   void extra_data_use_info_state::clear() {
      for (auto& id : this->list)
         id = 0;
      this->specials.patrol_ref_data.clear();
   }
   void extra_data_use_info_state::commit_to(form_stub_use_info_builder& uib) {
      for (auto id : this->list)
         //
         // Checking whether the ID is non-zero here is essential to fast execution.
         //
         // The (form_stub_use_info_builder::add_outbound_reference) function doesn't 
         // generate an outbound connection immediately; rather, it adds the ID to an 
         // internal list of pending connections, which can be committed or discarded 
         // all at once. We needed to do things that way for quest aliases. The list 
         // consists of a fixed part (std::array) and a resizable part (std::vector), 
         // such that we can get a resizable list if we need one while still being 
         // able to avoid vector-related overhead 90% of the time.
         //
         // Most extra-data types are going to be missing for any given REFR or CELL. 
         // If we just blindly add all IDs in this struct to the use info builder, 
         // we'll overflow its fixed part and rely on the resizable part; essentially, 
         // we're incurring tons of overhead in queueing connections-to-nothing, to 
         // the point of doing a bunch of vector resizing.
         //
         // When DovahKit is compiled in Debug and told to load Skyrim.esm, which has 
         // some two hundred thousand REFR records, NOT running this check costs us 
         // four entire seconds over what's normally a ten-second load.
         //
         if (id)
            uib.add_outbound_reference(id);
      //
      this->specials.patrol_ref_data.commit_to(uib);
   }
}