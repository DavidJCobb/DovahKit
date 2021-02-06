#include "Cell.h"
#include "_common_cpp.h"
#include "../notice_code_list.h"

namespace dovah::loaded_forms {
   void Cell::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (intfc.is_partial_record) // TESObjectCELL::LoadPartial is a no-op.
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
               subrecord.to_string(this->name);
               break;
            case 'XCLC':
               if (!loaded_cell_flags) {
                  detailed_notice warning;
                  warning.code = notice_code::cell_flags_not_yet_found;
                  warning.set_cause_form(this->stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  //
                  intfc.log_load_warning(warning);
                  //
                  // Cells without flags would theoretically default to being exteriors, so don't early-out here.
                  //
               } else if (this->cell_flags & cell_flag::interior) {
                  detailed_notice warning;
                  warning.code = notice_code::exterior_cell_data_in_interior_cell;
                  warning.set_cause_form(this->stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  //
                  intfc.log_load_warning(warning);
                  break;
               }
               subrecord.skip_bytes(sizeof(group_stub::gridX)); // form stubs store this information
               subrecord.skip_bytes(sizeof(group_stub::gridY)); // form stubs store this information
               subrecord.read(this->land_flags);
               break;
            case 'XCLL':
               if (!loaded_cell_flags) {
                  detailed_notice warning;
                  warning.code = notice_code::cell_flags_not_yet_found;
                  warning.set_cause_form(this->stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  //
                  intfc.log_load_warning(warning);
                  break;
               }
               if (!(this->cell_flags & cell_flag::interior)) {
                  detailed_notice warning;
                  warning.code = notice_code::interior_cell_data_in_exterior_cell;
                  warning.set_cause_form(this->stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  //
                  intfc.log_load_warning(warning);
                  break;
               }
               this->interior.lighting.load(subrecord, intfc);
               break;
            case 'LTMP':
               subrecord.read(this->interior.lighting_template_ID);
               intfc.log_load_warning( // if there's not actually anything to warn about, then this won't log anything
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::lighting_template, this->stub, this->interior.lighting_template_ID)
               );
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
               subrecord.read(this->water.height);
               break;
            case 'XNAM':
               subrecord.to_string(this->water.noise_texture);
               break;
            case 'OBND':
               this->has_object_bounds = true;
               this->object_bounds.load(subrecord, intfc);
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            default:
               if (this->extra_data.load(record, intfc) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
                  intfc.log_load_warning(
                     detailed_notice::warn_about_unrecognized_subrecord(subrecord.signature(), this->stub)
                  );
               }
               break;
         }
      }
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
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'LTMP':
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            default:
               if (components::extra_data_list::generate_use_info(record, uib) == components::extra_data_load_result::unrecognized) {
                  //
                  // Subrecord is not extra-data.
                  //
               }
               break;
         }
      }
   }
   void Cell::setup(const file_load_order& load_order) noexcept {
      if (!this->is_working_copy)
         cobb::edit_bit(this->cell_flags, cell_flag::interior, this->stub.groupInfo.parentFormID == 0);
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
   bool Cell::_clone_impl(Form* out) const noexcept {
      if (out->formType != form_type)
         return false;
      auto copy = (Cell*)out;
      //
      copy->name = this->name;
      copy->cell_flags = this->cell_flags;
      copy->land_flags = this->land_flags;
      copy->extra_data.clone_from(this->extra_data, *copy);
      copy->interior.lighting = this->interior.lighting;
      copy->interior.lighting_template_ID.set(*copy, this->interior.lighting_template_ID);
      copy->exterior.occlusion_data = this->exterior.occlusion_data;
      copy->exterior.max_height_data = this->exterior.max_height_data;
      copy->water.height = this->water.height;
      copy->water.noise_texture = this->water.noise_texture;
      copy->has_object_bounds = this->has_object_bounds;
      copy->object_bounds = this->object_bounds;
      copy->script_data.clone_from(this->script_data, *copy);
      return true;
   }
   bool Cell::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      if (record.flags() & tes_file_record_header::flag::partial) // TESObjectCELL::LoadPartial is a no-op
         return true;
      //
      bool is_exterior = this->stub.is_exterior_cell();
      //
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->cell_flags);
      DATA.close();
      if (is_exterior) {
         auto& XCLC = record.open_next_subrecord('XCLC');
         auto& stub = this->stub;
         XCLC.write(stub.groupInfo.gridX);
         XCLC.write(stub.groupInfo.gridY);
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
      record.write_formID_subrecord('LTMP', this->interior.lighting_template_ID);
      auto& XCLW = record.open_next_subrecord('XCLW');
      XCLW.write(this->water.height);
      XCLW.close();
      auto& XNAM = record.open_next_subrecord('XNAM');
      XNAM.write(this->water.noise_texture);
      XNAM.close();
      //
      this->extra_data.save(record, intfc);
      //
      if (this->has_object_bounds) {
         auto& subrecord = record.open_next_subrecord('OBND');
         this->object_bounds.save(subrecord, intfc);
         subrecord.close();
      }
      this->script_data.save(record, intfc); // VMAD (won't write anything if no scripts are attached)
      //
      return true;
   }
   void Cell::_clear_impl() noexcept {
      this->name.reset();
      this->cell_flags = 0;
      this->land_flags = 0;
      this->extra_data.clear(*this);
      this->interior.lighting_template_ID.set(*this, nullptr);
      this->exterior.max_height_data.clear();
      this->exterior.occlusion_data.clear();
      this->water.height = 0.0F;
      this->water.noise_texture.clear();
      this->has_object_bounds = false;
      this->object_bounds.clear();
      this->script_data.clear(*this);
   }
   void Cell::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->extra_data.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      //
      this->interior.lighting_template_ID.clear_if(*this, other);
   }
}