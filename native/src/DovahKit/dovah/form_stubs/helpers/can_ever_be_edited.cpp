#include "./can_ever_be_edited.h"
#include "../../data/hardcoded_form_ids.h"
#include "../../form_stub.h"

namespace dovah::form_stub_helpers {
   extern bool can_ever_be_edited(const form_stub& stub) {
      //
      // None-type stubs represent either bare TESForms used internally by the engine 
      // (e.g. the PapyrusPersistenceForm) or none-stubs. Neither of these can be saved 
      // to a file.
      //
      if (stub.form_type == form_type::none)
         return false;

      //
      // PlayerRef can't be edited. The player isn't part of any parent cell (and isn't 
      // supposed to be) within data files; ergo there's no parent CELL record to write 
      // their ACHR record to; ergo even if you were to make changes to the player, we 
      // would never save them. (And then we'd detect that the player ref wasn't saved, 
      // and try and fail to delete it, and then the editor would realize that it's in 
      // an inconsistent state and bail out.)
      //
      if (stub.formID == hardcoded_form_ids::PlayerRef)
         return false;

      return true;
   }
}