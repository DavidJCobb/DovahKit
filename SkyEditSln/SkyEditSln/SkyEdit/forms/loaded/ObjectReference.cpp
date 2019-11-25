#include "ObjectReference.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"

namespace LoadedForms {
   /*static*/ void ObjectReference::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               PapyrusScriptData::generateUseInfo(subrecord, stub);
               break;
            case 'NAME': // base form (subrecord signature is vestigial from Morrowind, which used editor IDs instead of form IDs)
            case 'LNAM': // lighting template
            case 'INAM': // imagespace (for imagespace modifier volumes)
            case 'XLRM': // location room marker (roombounds?)
            case 'XEMI': // emitted light
            case 'XESP': // enable parent (has four more bytes, but we don't need them)
            case 'XNDP': // door/navmesh linkage? (has four more bytes, but we don't need them)
            case 'XTEL': // door teleport (has 28 more bytes, but we don't need them)
            case 'XAPR': // activation parent (has four more bytes, but we don't need them)
            case 'XLIB': // leveled item base
            case 'XLRT': // LocRefType
            case 'XOWN': // owner
            case 'XCZC':
            case 'XEZN': // encounter zone
            case 'XMBR': // multibound ref
            case 'XPWR':
            case 'XATR': // attach ref // NOTE: at least one ref in Dawnguard.esm has an invalid form ID here
            case 'XLRL':
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               break;
            case 'XPOD': // load door teleport data (origin, destination)
            case 'XLKR': // location route (keyword, refr)
               if (subrecord.read(formID)) {
                  stub->add_outbound_reference(formID);
                  if (subrecord.read(formID))
                     stub->add_outbound_reference(formID);
               }
               break;
            case 'XLOC': // lock information (door/container)
               subrecord.skip_bytes(4);
               if (subrecord.read(formID))
                  stub->add_outbound_reference(formID);
               // has four more bytes, but we don't need them
               break;
            case 'PDTO': // same as in PACK
               {
                  uint32_t topicType; // if non-zero, then next four bytes is a signature e.g. 'HELO' for "Hello" subtype
                  if (subrecord.read(topicType) && topicType == 0)
                     if (subrecord.read(formID))
                        stub->add_outbound_reference(formID);
                     // else ignore the next four bytes
               }
               break;
            case 'EDID': // editor ID
            case 'XMBO': // model bounds
            case 'XPRM': // primitive
            case 'XRMR': // marker
            case 'SCHR': // DEPRECATED: ObScript header
            case 'SCDA': // DEPRECATED: ObScript compiled code
            case 'SCTX': // DEPRECATED: ObScript source code
            case 'SCRO': // DEPRECATED: ObScript ObjectReference
            case 'SCRV': // DEPRECATED: ObScript ObjectReference variable
            case 'XRGD':
            case 'XRDS': // radius
            case 'XLIG': // light data
            case 'XALP': // alpha cutoff
            case 'XSCL': // scale
            case 'XAPD': // activation parent flags
            case 'XCNT': // count (e.g. for preplaced items, especially arrows)
            case 'XCVL': // water current linear velocity
            case 'XCVR': // water current rotational velocity (TODO: verify)
            case 'XCZA':
            case 'XFVC': // favor cost
            case 'FNAM': // map marker flags
            case 'FULL': // name
            case 'TNAM': // map marker type
            case 'XHTW': // headtracking weight
            case 'XIS2': // Ignored By Sandbox flag?
            case 'XLCM':
            case 'XOCP': // occlusion plane data
            case 'XTRI': // primitive collision layer
            case 'ONAM':
            case 'XACT':
            case 'XWCN':
            case 'XWCU': // water current
            case 'XPRD': // patrol data idle time
            case 'XPPA':
            case 'DATA':
               break;
         }
      }
   }
}