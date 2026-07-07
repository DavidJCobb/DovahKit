#include "Cell.h"
#include "_common_cpp.h"
#include "../form_stub_addenda.h"
#include "components/extra_data/use_info_state.h"

#include "../notices/form_load_warnings/by_form_type/cell/cell_type_not_yet_known.h"
#include "../notices/form_load_warnings/by_form_type/cell/data_for_wrong_cell_type.h"

namespace dovah::loaded_forms {
   void Cell::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (intfc.is_partial_record) // TESObjectCELL::LoadPartial is a no-op.
         return;
      if (!intfc.is_winning_record)
         return;
      //
      form_id_t formID;
      bool      loaded_cell_flags = false;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.read(this->cell_flags);
               loaded_cell_flags = true;
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'XCLC':
               if (!loaded_cell_flags) {
                  notices::form_load_warnings::by_type::cell::cell_type_not_yet_known notice(
                     this->stub,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  //
                  // Cells without flags would theoretically default to being exteriors, so don't early-out here.
                  //
               } else if (this->cell_flags & cell_flag::interior) {
                  notices::form_load_warnings::by_type::cell::data_for_wrong_cell_type notice(
                     this->stub,
                     notices::form_load_warnings::by_type::cell::data_for_wrong_cell_type::cell_type::interior,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
               }
               subrecord.skip_bytes(sizeof(int32_t)); // XCLC grid X; form stubs store this information
               subrecord.skip_bytes(sizeof(int32_t)); // XCLC grid Y; form stubs store this information
               subrecord.read(this->land_flags);
               break;
            case 'XCLL':
               if (!loaded_cell_flags) {
                  notices::form_load_warnings::by_type::cell::cell_type_not_yet_known notice(
                     this->stub,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
               }
               if (!(this->cell_flags & cell_flag::interior)) {
                  notices::form_load_warnings::by_type::cell::data_for_wrong_cell_type notice(
                     this->stub,
                     notices::form_load_warnings::by_type::cell::data_for_wrong_cell_type::cell_type::exterior,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
                  break;
               }
               this->interior.lighting.load(subrecord, intfc);
               break;
            case 'LTMP':
               subrecord.read(this->interior.lighting_template);
               intfc.warn_if_ref_is_wrong_type(this->interior.lighting_template, form_type::lighting_template, subrecord.signature());
               break;
            case 'TVDT':
               this->exterior.occlusion_data.present = true;
               this->exterior.occlusion_data.bytes.resize(subrecord.size());
               subrecord.read(this->exterior.occlusion_data.bytes.data(), subrecord.size());
               break;
            case 'MHDT':
               if (subrecord.is_in_bounds(sizeof(float) + 32 * 32)) {
                  auto& mhdt = this->exterior.max_height_data;
                  subrecord.unchecked_read(mhdt.offset);
                  for (auto& row : mhdt.grid)
                     for (auto& col : row)
                        subrecord.unchecked_read(col);
                  mhdt.present = true;
               }
               break;
            case 'LNAM':
               subrecord.read(this->interior.lighting.inherit_flags);
               break;
            case 'XCLW':
               {
                  float value;
                  subrecord.read(value);
                  if (value >= inherit_water_height)
                     break;
                  this->water.height = round(value); // the game rounds to the nearest multiple of 1.0F
               }
               break;
            case 'XNAM':
               subrecord.read(this->water.noise_texture);
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               if (this->extra_data.load(record, intfc) == components::extra_data_list::load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
                  intfc.warn_on_unrecognized_subrecord(subrecord);
               }
               break;
         }
      }

      this->extra_data.post_load_validation(intfc);
   }
   /*static*/ void Cell::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (uib.is_partial_record) // TESObjectCELL::LoadPartial is a no-op.
         return;
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;
      //
      form_id_t lighting_template;
      components::extra_data_use_info_state eduis;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'LTMP':
               subrecord.read(lighting_template);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
            case 'FULL':
            case 'LNAM':
            case 'MHDT':
            case 'OBND':
            case 'TVDT':
            case 'XCLC':
            case 'XCLL':
            case 'XCLW':
            case 'XNAM':
               break;
            default:
               if (!components::extra_data_list::generate_use_info(record, uib, eduis)) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
      uib.add_outbound_reference(lighting_template);
      eduis.commit_to(uib);
   }
   void Cell::setup(const file_load_order& load_order) noexcept {
      if (!this->is_working_copy)
         cobb::edit_bit(this->cell_flags, cell_flag::interior, !this->stub.is_exterior_cell());
   }
   bool Cell::would_bethesda_compress() const noexcept {
      if (this->exterior.occlusion_data.present)
         //
         // Skyrim.esm only seems to compress CELLs if they contain TVDT...
         //
         return true;
      if (this->exterior.max_height_data.present)
         //
         // ...but I've seen the Creation Kit compress cells that contain MHDT as well.
         //
         return true;
      return false;
   }
   void Cell::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Cell*)out;
      //
      copy->name = this->name;
      copy->cell_flags = this->cell_flags;
      copy->land_flags = this->land_flags;
      copy->extra_data.clone_from(this->extra_data, *copy);
      copy->interior.lighting = this->interior.lighting;
      copy->interior.lighting_template.set(*copy, this->interior.lighting_template);
      copy->exterior.occlusion_data = this->exterior.occlusion_data;
      copy->exterior.max_height_data = this->exterior.max_height_data;
      copy->water.height = this->water.height;
      copy->water.noise_texture = this->water.noise_texture;
      copy->script_data.clone_from(this->script_data, *copy);
   }
   void Cell::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (record.flags() & tes_file_record_header::flag::partial) // TESObjectCELL::LoadPartial is a no-op
         return;
      //
      bool is_exterior = this->stub.is_exterior_cell();
      //
      this->script_data.save(record, intfc); // VMAD (won't write anything if no scripts are attached)
      if (!this->name.empty()) {
         auto& FULL = record.open_next_subrecord('FULL');
         FULL.write(this->name);
         FULL.close();
      }
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->cell_flags);
      DATA.close();
      if (is_exterior) {
         auto& XCLC = record.open_next_subrecord('XCLC');
         auto& stub = this->stub;
         int32_t x = 0;
         int32_t y = 0;
         stub.get_grid_coordinates(x, y);
         XCLC.write(x);
         XCLC.write(y);
         XCLC.write(this->land_flags);
         XCLC.close();
      } else {
         auto& XCLL = record.open_next_subrecord('XCLL');
         this->interior.lighting.save(XCLL, intfc);
         XCLL.close();
      }
      if (this->exterior.occlusion_data.present) {
         auto& data = this->exterior.occlusion_data;
         auto& TVDT = record.open_next_subrecord('TVDT');
         TVDT.write(data.bytes.data(), data.bytes.size());
         TVDT.close();
      }
      if (this->exterior.max_height_data.present) {
         auto& data = this->exterior.max_height_data;
         auto& MHDT = record.open_next_subrecord('MHDT');
         MHDT.write(data.offset);
         for (auto& row : data.grid)
            for (auto& col : row)
               MHDT.write(col);
         MHDT.close();
      }
      record.write_formID_subrecord('LTMP', this->interior.lighting_template, true);
      auto& XCLW = record.open_next_subrecord('XCLW');
      XCLW.write(this->water.height);
      XCLW.close();
      if (!this->water.noise_texture.empty()) {
         auto& XNAM = record.open_next_subrecord('XNAM');
         XNAM.write(this->water.noise_texture);
         XNAM.close();
      }
      //
      this->extra_data.save(record, intfc);
   }
   void Cell::_clear_impl() noexcept {
      this->name.reset();
      this->cell_flags = 0;
      this->land_flags = 0;
      this->extra_data.clear(*this);
      this->interior.lighting_template.set(*this, nullptr);
      this->exterior.max_height_data.clear();
      this->exterior.occlusion_data.clear();
      this->water.height = 0.0F;
      this->water.noise_texture.clear();
      this->script_data.clear(*this);
   }
   void Cell::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->extra_data.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      //
      this->interior.lighting_template.clear_if(*this, other);
   }
}