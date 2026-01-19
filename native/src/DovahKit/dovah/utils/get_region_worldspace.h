#pragma once
namespace dovah {
   class form_stub;
}

namespace dovah::utils {
   //
   // This is not as simple as checking REGN/WNAM. If a REGN has no WNAM but has in fact 
   // had region areas drawn in some worldspace, then you have to check the CELL/XCLR 
   // subrecords to determine its worldspace -- and the order in which those subrecords 
   // (not whole records!) load determines which worldspace "wins."
   //
   extern form_stub* get_region_worldspace(form_stub& stub);
}