#pragma once

namespace dovah {
   class form_stub;
}

namespace dovah::form_stub_helpers {
   //
   // Check if a form stub can ever be flagged as edited. Some stubs can't; for example, 
   // forms that are never saved to a file (e.g. PlayerRef, PapyrusPersistenceForm) can't 
   // be flagged as edited.
   //
   extern bool can_ever_be_edited(const form_stub&);
}