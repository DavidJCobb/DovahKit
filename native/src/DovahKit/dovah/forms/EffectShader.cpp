#include "EffectShader.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void EffectShader::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;

            case 'ICON':
               subrecord.read(this->membrane.fill.textures.main);
               break;
            case 'ICO2':
               subrecord.read(this->particle.textures.main);
               break;
            case 'NAM7':
               subrecord.read(this->membrane.fill.textures.holes);
               break;
            case 'NAM8':
               subrecord.read(this->membrane.fill.textures.palette);
               break;
            case 'NAM9':
               subrecord.read(this->particle.textures.palette);
               break;
            case 'DATA':
               subrecord.read(this->data_unk000);
               subrecord.read(this->membrane.blend.src);
               subrecord.read(this->membrane.blend.op);
               subrecord.read(this->membrane.blend.z_test);
               this->membrane.fill.color_keys[0].color.load(subrecord);
               subrecord.read(this->membrane.fill.alpha.times.fade_in);
               subrecord.read(this->membrane.fill.alpha.times.full);
               subrecord.read(this->membrane.fill.alpha.times.fade_out);
               subrecord.read(this->membrane.fill.alpha.ratios.persistent); // DATA+0x20
               subrecord.read(this->membrane.fill.alpha.pulse.amplitude);
               subrecord.read(this->membrane.fill.alpha.pulse.frequency);
               subrecord.read(this->membrane.fill.textures.speed.u);
               subrecord.read(this->membrane.fill.textures.speed.v); // DATA+0x30
               subrecord.read(this->membrane.edge.falloff);
               this->membrane.edge.color.load(subrecord);
               subrecord.read(this->membrane.edge.alpha.times.fade_in);
               subrecord.read(this->membrane.edge.alpha.times.full); // DATA+0x40
               subrecord.read(this->membrane.edge.alpha.times.fade_out);
               subrecord.read(this->membrane.edge.alpha.ratios.persistent);
               subrecord.read(this->membrane.edge.alpha.pulse.amplitude);
               subrecord.read(this->membrane.edge.alpha.pulse.frequency); // DATA+0x50
               subrecord.read(this->membrane.fill.alpha.ratios.full);
               subrecord.read(this->membrane.edge.alpha.ratios.full);
               subrecord.read(this->membrane.blend.dst);
               subrecord.read(this->particle.blend.src); // DATA+0x60
               subrecord.read(this->particle.blend.op);
               subrecord.read(this->particle.blend.z_test);
               subrecord.read(this->particle.blend.dst);
               subrecord.read(this->particle.behavior.spawn.times.ramp_up); // DATA+0x70
               subrecord.read(this->particle.behavior.spawn.times.full);
               subrecord.read(this->particle.behavior.spawn.times.ramp_down);
               subrecord.read(this->particle.behavior.spawn.counts.full);
               subrecord.read(this->particle.behavior.spawn.counts.persistent); // DATA+0x80
               subrecord.read(this->particle.behavior.lifetime.base);
               subrecord.read(this->particle.behavior.lifetime.variance);
               subrecord.read(this->particle.behavior.movement.initial_speed.base);
               subrecord.read(this->particle.behavior.movement.acceleration.along_normal); // DATA+0x90
               subrecord.read(this->particle.behavior.movement.initial_velocity.x);
               subrecord.read(this->particle.behavior.movement.initial_velocity.y);
               subrecord.read(this->particle.behavior.movement.initial_velocity.z);
               subrecord.read(this->particle.behavior.movement.acceleration.absolute.x); // DATA+0xA0
               subrecord.read(this->particle.behavior.movement.acceleration.absolute.y);
               subrecord.read(this->particle.behavior.movement.acceleration.absolute.z);
               subrecord.read(this->particle.scale_keys[0].scale);
               subrecord.read(this->particle.scale_keys[1].scale); // DATA+0xB0
               subrecord.read(this->particle.scale_keys[0].time);
               subrecord.read(this->particle.scale_keys[1].time);
               {  // DATA+0xBC
                  auto& list = this->particle.color_keys;
                  for (auto& item : list)
                     item.color.load(subrecord); // DATA+0xBC, C0, C4
                  for (auto& item : list)
                     subrecord.read(item.alpha); // DATA+0xC8, CC, D0
                  for (auto& item : list)
                     subrecord.read(item.time); // DATA+0xD4, D8, DC
               }
               subrecord.read(this->particle.behavior.movement.initial_speed.variance); // DATA+0xE0
               subrecord.read(this->particle.behavior.movement.initial_rotation.base);
               subrecord.read(this->particle.behavior.movement.initial_rotation.variance);
               subrecord.read(this->particle.behavior.movement.rotation_speed.base);
               subrecord.read(this->particle.behavior.movement.rotation_speed.variance); // DATA+0xF0
               if (auto& form = this->particle.debris.form; subrecord.read(form)) {
                  intfc.warn_if_ref_is_wrong_type(form, form_type::debris, subrecord.signature());
               }
               subrecord.read(this->membrane.holes.start_time);
               subrecord.read(this->membrane.holes.end_time);
               subrecord.read(this->membrane.holes.start_value); // DATA+0x100
               subrecord.read(this->membrane.holes.end_value);
               subrecord.read(this->data_unk108); // ???
               this->data_unk10C.load(subrecord);
               subrecord.read(this->particle.behavior.movement.explosion_wind_speed); // DATA+0x110
               subrecord.read(this->particle.textures.count.u);
               subrecord.read(this->particle.textures.count.v);
               subrecord.read(this->particle.debris.times.fade_in);
               subrecord.read(this->particle.debris.times.fade_out); // DATA+0x120
               subrecord.read(this->particle.debris.scales.start);
               subrecord.read(this->particle.debris.scales.end);
               subrecord.read(this->particle.debris.times.scale_in);
               subrecord.read(this->particle.debris.times.scale_out); // DATA+0x130
               if (auto& form = this->ambient_sound; subrecord.read(form)) { // DATA+0x134
                  intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::sound_descriptor, form_type::sound }, subrecord.signature());
               }
               {
                  auto& list = this->membrane.fill.color_keys;
                  for (size_t i = 1; i < list.size(); ++i) // DATA+0x138, 13C
                     list[i].color.load(subrecord);
                  for (auto& item : list) // DATA+0x140, 144, 148
                     subrecord.read(item.alpha);
                  for (auto& item : list) // DATA+0x14C, 150, 154
                     subrecord.read(item.time);
               }
               subrecord.read(this->data_unk158); // DATA+0x158
               subrecord.read(this->particle.behavior.movement.initial_position.base);
               subrecord.read(this->particle.behavior.movement.initial_position.variance); // DATA+0x160
               subrecord.read(this->particle.textures.animation.start_frame.base);
               subrecord.read(this->particle.textures.animation.start_frame.variance);
               subrecord.read(this->particle.textures.animation.end_frame);
               subrecord.read(this->particle.textures.animation.loop_start_frame.base); // DATA+0x170
               subrecord.read(this->particle.textures.animation.loop_start_frame.variance);
               subrecord.read(this->particle.textures.animation.frame_count.base);
               subrecord.read(this->particle.textures.animation.frame_count.variance);
               subrecord.read(this->flags); // DATA+0x180
               subrecord.read(this->membrane.fill.textures.scale.u);
               subrecord.read(this->membrane.fill.textures.scale.v);
               subrecord.read(this->particle.behavior.spawn.scene_graph_emit_depth_limit); // DATA+0x18C
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void EffectShader::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;
      
      form_id_t ambient_sound;
      form_id_t particle_debris;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'DATA':
               subrecord.skip_bytes(0xF4);
               subrecord.read(particle_debris); // F4
               subrecord.skip_bytes(0x134 - 0xF8);
               subrecord.read(ambient_sound);
               break;
         }
      }
      uib.add_outbound_reference(ambient_sound);
      uib.add_outbound_reference(particle_debris);
   }
   void EffectShader::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (EffectShader*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->ambient_sound.set(*copy, this->ambient_sound);
      copy->flags = this->flags;

      copy->membrane = this->membrane;

      copy->particle.behavior = this->particle.behavior;
      copy->particle.blend = this->particle.blend;
      copy->particle.color_keys = this->particle.color_keys;
      {
         auto& src = this->particle.debris;
         auto& dst = copy->particle.debris;
         dst.form.set(*copy, src.form);
         dst.scales = src.scales;
         dst.times  = src.times;
      }
      copy->particle.scale_keys = this->particle.scale_keys;
      copy->particle.textures = this->particle.textures;

      copy->data_unk000 = this->data_unk000;
      copy->data_unk108 = this->data_unk108;
      copy->data_unk10C = this->data_unk10C;
      copy->data_unk158 = this->data_unk158;
   }
   void EffectShader::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      record.write_string_subrecord('ICON', this->membrane.fill.textures.main);
      record.write_string_subrecord('ICO2', this->particle.textures.main);
      record.write_string_subrecord('NAM7', this->membrane.fill.textures.holes);
      record.write_string_subrecord('NAM8', this->membrane.fill.textures.palette);
      record.write_string_subrecord('NAM9', this->particle.textures.palette);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data_unk000);
         subrecord.write(this->membrane.blend.src);
         subrecord.write(this->membrane.blend.op);
         subrecord.write(this->membrane.blend.z_test);
         this->membrane.fill.color_keys[0].color.save(subrecord);
         subrecord.write(this->membrane.fill.alpha.times.fade_in);
         subrecord.write(this->membrane.fill.alpha.times.full);
         subrecord.write(this->membrane.fill.alpha.times.fade_out);
         subrecord.write(this->membrane.fill.alpha.ratios.persistent); // DATA+0x20
         subrecord.write(this->membrane.fill.alpha.pulse.amplitude);
         subrecord.write(this->membrane.fill.alpha.pulse.frequency);
         subrecord.write(this->membrane.fill.textures.speed.u);
         subrecord.write(this->membrane.fill.textures.speed.v); // DATA+0x30
         subrecord.write(this->membrane.edge.falloff);
         this->membrane.edge.color.save(subrecord);
         subrecord.write(this->membrane.edge.alpha.times.fade_in);
         subrecord.write(this->membrane.edge.alpha.times.full); // DATA+0x40
         subrecord.write(this->membrane.edge.alpha.times.fade_out);
         subrecord.write(this->membrane.edge.alpha.ratios.persistent);
         subrecord.write(this->membrane.edge.alpha.pulse.amplitude);
         subrecord.write(this->membrane.edge.alpha.pulse.frequency); // DATA+0x50
         subrecord.write(this->membrane.fill.alpha.ratios.full);
         subrecord.write(this->membrane.edge.alpha.ratios.full);
         subrecord.write(this->membrane.blend.dst);
         subrecord.write(this->particle.blend.src); // DATA+0x60
         subrecord.write(this->particle.blend.op);
         subrecord.write(this->particle.blend.z_test);
         subrecord.write(this->particle.blend.dst);
         subrecord.write(this->particle.behavior.spawn.times.ramp_up); // DATA+0x70
         subrecord.write(this->particle.behavior.spawn.times.full);
         subrecord.write(this->particle.behavior.spawn.times.ramp_down);
         subrecord.write(this->particle.behavior.spawn.counts.full);
         subrecord.write(this->particle.behavior.spawn.counts.persistent); // DATA+0x80
         subrecord.write(this->particle.behavior.lifetime.base);
         subrecord.write(this->particle.behavior.lifetime.variance);
         subrecord.write(this->particle.behavior.movement.initial_speed.base);
         subrecord.write(this->particle.behavior.movement.acceleration.along_normal); // DATA+0x90
         subrecord.write(this->particle.behavior.movement.initial_velocity.x);
         subrecord.write(this->particle.behavior.movement.initial_velocity.y);
         subrecord.write(this->particle.behavior.movement.initial_velocity.z);
         subrecord.write(this->particle.behavior.movement.acceleration.absolute.x); // DATA+0xA0
         subrecord.write(this->particle.behavior.movement.acceleration.absolute.y);
         subrecord.write(this->particle.behavior.movement.acceleration.absolute.z);
         subrecord.write(this->particle.scale_keys[0].scale);
         subrecord.write(this->particle.scale_keys[1].scale); // DATA+0xB0
         subrecord.write(this->particle.scale_keys[0].time);
         subrecord.write(this->particle.scale_keys[1].time);
         {  // DATA+0xBC
            auto& list = this->particle.color_keys;
            for (auto& item : list)
               item.color.save(subrecord); // DATA+0xBC, C0, C4
            for (auto& item : list)
               subrecord.write(item.alpha); // DATA+0xC8, CC, D0
            for (auto& item : list)
               subrecord.write(item.time); // DATA+0xD4, D8, DC
         }
         subrecord.write(this->particle.behavior.movement.initial_speed.variance); // DATA+0xE0
         subrecord.write(this->particle.behavior.movement.initial_rotation.base);
         subrecord.write(this->particle.behavior.movement.initial_rotation.variance);
         subrecord.write(this->particle.behavior.movement.rotation_speed.base);
         subrecord.write(this->particle.behavior.movement.rotation_speed.variance); // DATA+0xF0
         subrecord.write(this->particle.debris.form);
         subrecord.write(this->membrane.holes.start_time);
         subrecord.write(this->membrane.holes.end_time);
         subrecord.write(this->membrane.holes.start_value); // DATA+0x100
         subrecord.write(this->membrane.holes.end_value);
         subrecord.write(this->data_unk108); // ???
         this->data_unk10C.save(subrecord);
         subrecord.write(this->particle.behavior.movement.explosion_wind_speed); // DATA+0x110
         subrecord.write(this->particle.textures.count.u);
         subrecord.write(this->particle.textures.count.v);
         subrecord.write(this->particle.debris.times.fade_in);
         subrecord.write(this->particle.debris.times.fade_out); // DATA+0x120
         subrecord.write(this->particle.debris.scales.start);
         subrecord.write(this->particle.debris.scales.end);
         subrecord.write(this->particle.debris.times.scale_in);
         subrecord.write(this->particle.debris.times.scale_out); // DATA+0x130
         subrecord.write(this->ambient_sound);
         {
            auto& list = this->membrane.fill.color_keys;
            for (size_t i = 1; i < list.size(); ++i) // DATA+0x138, 13C
               list[i].color.save(subrecord);
            for (auto& item : list) // DATA+0x140, 144, 148
               subrecord.write(item.alpha);
            for (auto& item : list) // DATA+0x14C, 150, 154
               subrecord.write(item.time);
         }
         subrecord.write(this->data_unk158); // DATA+0x158
         subrecord.write(this->particle.behavior.movement.initial_position.base);
         subrecord.write(this->particle.behavior.movement.initial_position.variance); // DATA+0x160
         subrecord.write(this->particle.textures.animation.start_frame.base);
         subrecord.write(this->particle.textures.animation.start_frame.variance);
         subrecord.write(this->particle.textures.animation.end_frame);
         subrecord.write(this->particle.textures.animation.loop_start_frame.base); // DATA+0x170
         subrecord.write(this->particle.textures.animation.loop_start_frame.variance);
         subrecord.write(this->particle.textures.animation.frame_count.base);
         subrecord.write(this->particle.textures.animation.frame_count.variance);
         subrecord.write(this->flags); // DATA+0x180
         subrecord.write(this->membrane.fill.textures.scale.u);
         subrecord.write(this->membrane.fill.textures.scale.v);
         subrecord.write(this->particle.behavior.spawn.scene_graph_emit_depth_limit); // DATA+0x18C
         subrecord.close();
      }
   }
   void EffectShader::_clear_impl() noexcept {
      this->script_data.clear(*this);

      this->ambient_sound.set(*this, nullptr);
      this->flags = this->flags;

      this->membrane = this->membrane;

      this->particle.behavior = this->particle.behavior;
      this->particle.blend = this->particle.blend;
      this->particle.color_keys = this->particle.color_keys;
      {
         auto& src = this->particle.debris;
         auto& dst = this->particle.debris;
         dst.form.set(*this, nullptr);
         dst.scales = {};
         dst.times = {};
      }
      this->particle.scale_keys = {};
      this->particle.textures = {};

      this->data_unk000 = {};
      this->data_unk108 = {};
      this->data_unk10C = {};
      this->data_unk158 = {};
   }
   void EffectShader::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->ambient_sound.clear_if(*this, other);
      this->particle.debris.form.clear_if(*this, other);
   }
}