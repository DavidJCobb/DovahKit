#pragma once
#include "./common_empty.h"
#include "./common_fundamental.h"

namespace dovah::loaded_forms::components::extra_data_types::deprecated {
   class XCET : public common_empty<XCET, 'XCET'> {
      // The game skips loading this subrecord. In FO3, it was a Decal Reference.
   };
   class XDCR : public common_empty<XDCR, 'XDCR'> {
      // The game skips loading this subrecord. In FO3, it was a Decal Reference.
   };
   class XEDL : public common_empty<XEDL, 'XEDL'> {
   };
   class XENC : public common_empty<XENC, 'XENC'> {
   };
   class XHRS : public common_empty<XHRS, 'XHRS'> {
      // The game skips loading this subrecord. In TES4, it indicated an actor's hors eand worked the way XHOR does now.
   };
   class XIBS : public common_empty<XIBS, 'XIBS'> {
      // The game skips loading this subrecord. In FO3, it apparently worked the same way as XIS2.
   };
   class XLMB : public common_empty<XLMB, 'XLMB'> {
   };
   class XNVP : public common_empty<XNVP, 'XNVP'> {
   };
   class XPCI : public common_empty<XPCI, 'XPCI'> {
      // The game skips loading this subrecord. It originates from TES4.
   };
   class XRAD : public common_empty<XRAD, 'XRAD'> {
      // The game skips loading this subrecord. In FO3, it was a float indicating an amount of radiation.
   };
   class XRDO : public common_empty<XRDO, 'XRDO'> {
      // The game skips loading this subrecord. In FO3, it was radio data.
   };
   class XROO : public common_empty<XROO, 'XROO'> {
   };
   class XSED : public common_fundamental<XSED, 'XSED', uint32_t> {
   };
   class XSOL : public common_empty<XSOL, 'XSOL'> {
      // The game skips loading this subrecord. In TES4, it indicated the contained soul size in a ref.
   };
   class XUSE : public common_empty<XUSE, 'XUSE'> {
   };
   class XWLT : public common_empty<XWLT, 'XWLT'> {
   };
   class XWNT : public common_empty<XWNT, 'XWNT'> {
   };
}