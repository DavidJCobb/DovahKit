#include "ShaderParticleGeometry.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void ShaderParticleGeometry::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      /*
      
         For some reason, the game and CK store all of the fields in DATA 
         as an array of untyped dwords. Obviously, I've no interest in 
         doing the same.

         My blind guess is that the game engine just treats this as opaque 
         data to spew into a VRAM buffer for the shader to use, and only 
         the CK's UI really cares what any of the data actually is.

      */

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'DATA':
               subrecord.read(this->gravity_velocity);
               subrecord.read(this->rotation.velocity);
               subrecord.read(this->particles.size.x);
               subrecord.read(this->particles.size.y);
               subrecord.read(this->center_offset.min);
               subrecord.read(this->center_offset.max);
               subrecord.read(this->rotation.initial_range);
               subrecord.read(this->subtexture_count.x);
               subrecord.read(this->subtexture_count.y);
               subrecord.read(this->type);
               subrecord.read(this->box_size);
               subrecord.read(this->particles.density);
               break;
            case 'ICON':
               subrecord.read(this->particles.texture);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }

      //
      // That said, they do some corrections post-load, accessing data 
      // fields by index.
      //
      if (auto& v = this->subtexture_count.x; v < 1)
         v = 1;
      if (auto& v = this->subtexture_count.y; v < 1)
         v = 1;
      if (auto& v = this->box_size; v < 1)
         v = 4096;
      if (auto& v = this->particles.density; v < 1)
         v = 1;
   }
   /*static*/ void ShaderParticleGeometry::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
         }
      }
   }
   void ShaderParticleGeometry::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (ShaderParticleGeometry*)out;

      copy->script_data.clone_from(this->script_data, *copy);
      copy->center_offset = this->center_offset;
      copy->gravity_velocity = this->gravity_velocity;
      copy->particles = this->particles;
      copy->rotation = this->rotation;
      copy->subtexture_count = this->subtexture_count;
      copy->type = this->type;
   }
   void ShaderParticleGeometry::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->gravity_velocity);
         subrecord.write(this->rotation.velocity);
         subrecord.write(this->particles.size.x);
         subrecord.write(this->particles.size.y);
         subrecord.write(this->center_offset.min);
         subrecord.write(this->center_offset.max);
         subrecord.write(this->rotation.initial_range);
         subrecord.write(this->subtexture_count.x);
         subrecord.write(this->subtexture_count.y);
         subrecord.write(this->type);
         subrecord.write(this->box_size);
         subrecord.write(this->particles.density);
         subrecord.close();
      }
      if (!this->particles.texture.empty())
         record.write_string_subrecord('ICON', this->particles.texture);
   }
   void ShaderParticleGeometry::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->box_size = 4096;
      this->center_offset = {};
      this->gravity_velocity = 0;
      this->particles = {};
      this->rotation = {};
      this->subtexture_count = {};
      this->type = particle_type::rain;
   }
   void ShaderParticleGeometry::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
   }
}