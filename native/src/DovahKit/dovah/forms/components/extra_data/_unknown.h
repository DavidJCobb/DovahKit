#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra::unknown {
   class XCZA : public empty_extra_data<'XCZA', extra_data_type::unknown_xcza> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XCZC : public formID_extra_data<'XCZC', extra_data_type::unknown_xczc> { // appears on REFR
      // The form should be a CELL.
      // The game skips loading this subrecord.
   };
   class XCZR : public formID_extra_data<'XCZR', extra_data_type::unknown_xczr> {
      // The form should be a REFR.
      // The game skips loading this subrecord.
   };
   class XEDL : public empty_extra_data<'XEDL', extra_data_type::unknown_xedl> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XENC : public empty_extra_data<'XENC', extra_data_type::unknown_xenc> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XLMB : public empty_extra_data<'XLMB', extra_data_type::unknown_xlmb> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XNVP : public empty_extra_data<'XNVP', extra_data_type::unknown_xnvp> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XPSL : public empty_extra_data<'XPSL', extra_data_type::unknown_xpsl> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XROO : public empty_extra_data<'XROO', extra_data_type::unknown_xroo> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XUSE : public empty_extra_data<'XUSE', extra_data_type::unknown_xuse> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XWCS : public empty_extra_data<'XWCS', extra_data_type::unknown_xwcs> {
      // The game skips loading this subrecord.
   };
   class XWLT : public empty_extra_data<'XWLT', extra_data_type::unknown_xwlt> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
   class XWNT : public empty_extra_data<'XWNT', extra_data_type::unknown_xwnt> {
      // The game skips loading this subrecord, and no one's ever seen it before.
   };
}

namespace dovah::loaded_forms::components::extra::deprecated {
   class XDCR : public empty_extra_data<'XDCR', extra_data_type::deprecated_xdcr> {
      // The game skips loading this subrecord. In FO3, it was a Decal Reference.
   };
   class XHRS : public empty_extra_data<'XHRS', extra_data_type::deprecated_xhrs> {
      // The game skips loading this subrecord. In TES4, it indicated an actor's hors eand worked the way XHOR does now.
   };
   class XIBS : public empty_extra_data<'XIBS', extra_data_type::deprecated_xibs> {
      // The game skips loading this subrecord. In FO3, it apparently worked the same way as XIS2.
   };
   class XPCI : public empty_extra_data<'XPCI', extra_data_type::deprecated_xpci> {
      // The game skips loading this subrecord. It originates from TES4.
   };
   class XRAD : public empty_extra_data<'XRAD', extra_data_type::deprecated_xrad> {
      // The game skips loading this subrecord. In FO3, it was a float indicating an amount of radiation.
   };
   class XRDO : public empty_extra_data<'XRDO', extra_data_type::deprecated_xrdo> {
      // The game skips loading this subrecord. In FO3, it was radio data.
   };
   class XSED : public basic_extra_data {
      public:
         static constexpr uint32_t signature = 'XSED';
         //
         uint32_t value = 0;
         //
         virtual extra_data_type get_type() const noexcept { return extra_data_type::deprecated_xsed; };
         virtual load_result load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) override;
         virtual void save(tes_record_writer&) override;
         //
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&) {}
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept override;
   };
   class XSOL : public empty_extra_data<'XSOL', extra_data_type::deprecated_xsol> {
      // The game skips loading this subrecord. In TES4, it indicated the contained soul size in a ref.
   };
}