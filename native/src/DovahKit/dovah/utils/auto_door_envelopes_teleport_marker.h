#pragma once
namespace dovah {
   namespace loaded_forms {
      class ObjectReference;
   }
   class form_stub;
}

namespace dovah::utils {
   //
   // Is this ref a load door leading to an Automatic door and, if so, 
   // is the teleport marker on that side within the door's bounds?
   //
   extern bool auto_door_envelopes_teleport_marker(loaded_forms::ObjectReference& ref);
   extern bool auto_door_envelopes_teleport_marker(form_stub& ref);
}