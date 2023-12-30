#pragma once

namespace dovah::loaded_forms::components::papyrus {
   //
   // Enumeration applied to BoundScript data in the VMAD subrecord. Note that a single 
   // ScriptObject as applied to a REFR can be influenced by a BoundScript entry on the 
   // base form and a BoundScript entry on the REFR itself.   Each BoundScript's status 
   // is meaningful on its own, but the game can also compute the status of the pair as 
   // a whole.
   // 
   // * When a BoundScript is  only present on the REFR,  the pair's status is either 3 
   //   or 0. Values other than 3 are coerced to 0.
   // 
   // * When a BoundScript is only present on the non-REFR, the pair's status is either 
   //   3 or 2. Values other than 3 are coerced to 2.
   // 
   //   * The Creation Kit expects the non-REFR to have a status of 0 in this case.
   // 
   // * When a BoundScript is present on both forms, the pair's status is the status of 
   //   the REFR BoundScript, taken verbatim.
   // 
   // Additionally, the following behaviors occur when using unusual script statuses in-
   // game:
   // 
   // * If the base form and REFR both have the  same script listed in their respective 
   //   VMAD subrecords, and the REFR-side BoundScript has status 0, then the base form 
   //   BoundScript is not inherited from;  that specific REFR acts as if the script is 
   //   not bound on the base form at all. Properties  that aren't set on the REFR will 
   //   get the defaults  for their types (e.g. floats = 0.0)  rather than pulling from 
   //   the base form.
   // 
   //   * This can happen in the CK, if you attach the script to a REFR and then go and 
   //     attach it to the base form.  The fact that the game handles it this way means 
   //     that that  specific REFR will behave  the same way before and after  you edit 
   //     its base form.
   // 
   // * If the base form and REFR both have the  same script listed in their respective 
   //   VMAD subrecords, and the REFR-side BoundScript has status 2, then the game acts 
   //   like that BoundScript doesn't exist, and uses only the base-side BoundScript.
   //
   // The Creation Kit displays the following strings for each status:
   // 
   //    0: "Scripted added locally"
   //       (or, if properties are set, "Script added and edited locally")
   //    1: "Script inherited and edited locally"
   //    2: "Script inherited from parent"
   //    3: "Script inherited and deleted locally"
   //
   enum class script_status {
      defined_locally = 0, // can appear in ESPs
      overrides_base  = 1, // can appear in ESPs. base game and DLCs only use it ref-side; meaningless base-side
      defined_on_base = 2, // computed at run-time; not sensible for serialization
      removed         = 3, // can appear in ESPs. only makes sense on REFRs removing a base-side script (otherwise, just don't even list the BoundScript in VMAD)
   };
}