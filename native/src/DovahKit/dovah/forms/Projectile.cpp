#include "Projectile.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Projectile::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case components::object_bounds::subrecord:
               this->bounds.load(subrecord, intfc);
               break;
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               if (!this->destruction_data.has_value())
                  this->destruction_data.emplace();
               this->destruction_data.value().load(subrecord, intfc);
               break;
            case 'MODL':
            case 'MODS':
            case 'MODT':
            case 'MOSD':
               this->model.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DATA':
               subrecord.read(this->flags);
               subrecord.read(this->type);
               subrecord.read(this->gravity);
               subrecord.read(this->speed);
               subrecord.read(this->range);
               if (auto& form = this->light; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::light, subrecord.signature());
               if (auto& form = this->muzzle_flash.light; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::light, subrecord.signature());
               subrecord.read(this->tracer_chance);
               subrecord.read(this->explosion.alt_trigger.proximity);
               subrecord.read(this->explosion.alt_trigger.timer);
               if (auto& form = this->explosion.form; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::explosion, subrecord.signature());
               if (auto& form = this->sounds.flyby; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               subrecord.read(this->muzzle_flash.duration);
               subrecord.read(this->fade_duration);
               subrecord.read(this->impact_force);
               if (auto& form = this->sounds.countdown; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               if (auto& form = this->sounds.disarm; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::sound_descriptor, subrecord.signature());
               if (auto& form = this->default_weapon_source; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::weapon, subrecord.signature());
               subrecord.read(this->cone_spread);
               subrecord.read(this->collision_radius);
               subrecord.read(this->lifetime);
               subrecord.read(this->relaunch_interval);
               if (auto& form = this->decal; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::texture_set, subrecord.signature());
               if (auto& form = this->collision_layer; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::collision_layer, subrecord.signature());
               break;
            case 'NAM1':
               if (subrecord.size() == 0)
                  break;
               this->muzzle_flash.model.load_model_path(subrecord, intfc);
               break;
            case 'NAM2':
               this->muzzle_flash.model.load_precached_info(subrecord, intfc);
               break;
            case 'VNAM':
               subrecord.read(this->loudness);
               if ((std::underlying_type_t<detection_loudness>)this->loudness > 4) {
                  //
                  // This is the CK's error correction, done without any user-facing warning, but... what is 
                  // value 4? As of this writing, it's not known to correspond to anything. Maybe Bethesda 
                  // wrote the check improperly?
                  //
                  this->loudness = detection_loudness::normal;
               }
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Projectile::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;

      components::destruction_stage_data::use_info_builder destruction_uib(uib);
      form_id_t collision_layer;
      form_id_t decal;
      form_id_t default_weapon_source;
      struct {
         form_id_t form;
      } explosion;
      form_id_t light;
      struct {
         form_id_t light;
      } muzzle_flash;
      struct {
         form_id_t flyby;
         form_id_t countdown;
         form_id_t disarm;
      } sounds;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case components::destruction_stage_data::subrecord_header:
            case components::destruction_stage_data::subrecord_stage_data:
            case components::destruction_stage_data::subrecord_model_path:
            case components::destruction_stage_data::subrecord_model_hashes:
            case components::destruction_stage_data::subrecord_model_swaps:
            case components::destruction_stage_data::subrecord_terminator:
               components::destruction_stage_data::generate_use_info(subrecord, destruction_uib);
               break;
            case components::object_bounds::subrecord:
               components::object_bounds::generate_use_info(subrecord, uib);
               break;
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'DATA':
               subrecord.skip_bytes(
                  sizeof(flags) +
                  sizeof(type) +
                  sizeof(gravity) +
                  sizeof(speed) +
                  sizeof(range)
               );
               subrecord.read(light);
               subrecord.read(muzzle_flash.light);
               subrecord.skip_bytes(
                  sizeof(tracer_chance) +
                  sizeof(float) + // explosion.alt_trigger.proximity
                  sizeof(float)   // explosion.alt_trigger.trigger
               );
               subrecord.read(explosion.form);
               subrecord.read(sounds.flyby);
               subrecord.skip_bytes(
                  sizeof(float) + // muzzle_flash.duration
                  sizeof(fade_duration) +
                  sizeof(impact_force)
               );
               subrecord.read(sounds.countdown);
               subrecord.read(sounds.disarm);
               subrecord.read(default_weapon_source);
               subrecord.skip_bytes(
                  sizeof(cone_spread) +
                  sizeof(collision_radius) +
                  sizeof(lifetime) +
                  sizeof(relaunch_interval)
               );
               subrecord.read(decal);
               subrecord.read(collision_layer);
               break;
         }
      }
      uib.add_outbound_reference(collision_layer);
      uib.add_outbound_reference(decal);
      uib.add_outbound_reference(default_weapon_source);
      uib.add_outbound_reference(explosion.form);
      uib.add_outbound_reference(light);
      uib.add_outbound_reference(muzzle_flash.light);
      uib.add_outbound_reference(sounds.flyby);
      uib.add_outbound_reference(sounds.countdown);
      uib.add_outbound_reference(sounds.disarm);
      destruction_uib.done();
   }
   void Projectile::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Projectile*)out;
      
      copy->bounds = this->bounds;
      {
         auto& src_opt = this->destruction_data;
         auto& dst_opt = copy->destruction_data;
         if (dst_opt.has_value()) {
            dst_opt.value().clear(*copy);
            dst_opt = {};
         }
         if (src_opt.has_value()) {
            dst_opt.emplace();
            dst_opt.value().clone_from(src_opt.value(), *copy);
         }
      }
      copy->model.clone_from(this->model);
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;

      copy->flags = this->flags;
      copy->type  = this->type;
      copy->cone_spread = this->cone_spread;
      copy->collision_radius = this->collision_radius;
      copy->fade_duration = this->fade_duration;
      copy->gravity = this->gravity;
      copy->lifetime = this->lifetime;
      copy->impact_force = this->impact_force;
      copy->range = this->range;
      copy->relaunch_interval = this->relaunch_interval;
      copy->speed = this->speed;
      copy->tracer_chance = this->tracer_chance;

      copy->collision_layer.set(*copy, this->collision_layer);
      copy->decal.set(*copy, this->decal);
      copy->default_weapon_source.set(*copy, this->default_weapon_source);
      copy->explosion.alt_trigger = this->explosion.alt_trigger;
      copy->explosion.form.set(*copy, this->explosion.form);
      copy->light.set(*copy, this->light);
      copy->loudness = this->loudness;
      copy->muzzle_flash.duration = this->muzzle_flash.duration;
      copy->muzzle_flash.light.set(*copy, this->muzzle_flash.light);
      copy->muzzle_flash.model.clone_from(this->muzzle_flash.model);
      copy->sounds.flyby.set(*copy, this->sounds.flyby);
      copy->sounds.countdown.set(*copy, this->sounds.countdown);
      copy->sounds.disarm.set(*copy, this->sounds.disarm);
   }
   void Projectile::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& OBND = record.open_next_subrecord(components::object_bounds::subrecord);
      this->bounds.save(OBND, intfc);
      OBND.close();
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->name);
      FULL.close();
      this->model.save(record, intfc, 'MODL', 'MODT');
      if (this->destruction_data.has_value())
         this->destruction_data.value().save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->flags);
         subrecord.write(this->type);
         subrecord.write(this->gravity);
         subrecord.write(this->speed);
         subrecord.write(this->range);
         subrecord.write(this->light);
         subrecord.write(this->muzzle_flash.light);
         subrecord.write(this->tracer_chance);
         subrecord.write(this->explosion.alt_trigger.proximity);
         subrecord.write(this->explosion.alt_trigger.timer);
         subrecord.write(this->explosion.form);
         subrecord.write(this->sounds.flyby);
         subrecord.write(this->muzzle_flash.duration);
         subrecord.write(this->fade_duration);
         subrecord.write(this->impact_force);
         subrecord.write(this->sounds.countdown);
         subrecord.write(this->sounds.disarm);
         subrecord.write(this->default_weapon_source);
         subrecord.write(this->cone_spread);
         subrecord.write(this->collision_radius);
         subrecord.write(this->lifetime);
         subrecord.write(this->relaunch_interval);
         subrecord.write(this->decal);
         subrecord.write(this->collision_layer);
         subrecord.close();
      }
      {
         auto& model = this->muzzle_flash.model;
         if (!model.model_path.empty()) {
            auto& subrecord = record.open_next_subrecord('NAM1');
            model.save_model_path(subrecord, intfc);
            subrecord.close();
         }
         if (model.has_precached_info()) {
            auto& subrecord = record.open_next_subrecord('NAM2');
            model.save_precached_info(subrecord, intfc);
            subrecord.close();
         }
      }
      {
         auto& subrecord = record.open_next_subrecord('VNAM');
         subrecord.write(this->loudness);
         subrecord.close();
      }
   }
   void Projectile::_clear_impl() noexcept {
      this->bounds.clear();
      if (this->destruction_data.has_value()) {
         this->destruction_data.value().clear(*this);
         this->destruction_data = {};
      }
      this->model.clear();
      this->script_data.clear(*this);

      this->name.reset();

      this->collision_layer.set(*this, nullptr);
      this->decal.set(*this, nullptr);
      this->default_weapon_source.set(*this, nullptr);
      this->explosion.form.set(*this, nullptr);
      this->light.set(*this, nullptr);
      this->muzzle_flash.light.set(*this, nullptr);
      this->muzzle_flash.model.clear();
      this->sounds.countdown.set(*this, nullptr);
      this->sounds.disarm.set(*this, nullptr);
      this->sounds.flyby.set(*this, nullptr);

      this->cone_spread = 0;
      this->collision_radius = 10;
      this->explosion.alt_trigger = {};
      this->fade_duration = 0.5;
      this->flags = 0;
      this->gravity = 1000;
      this->impact_force = 0;
      this->lifetime = 0;
      this->loudness = detection_loudness::normal;
      this->muzzle_flash.duration = 0;
      this->range = 0;
      if (const auto* dfn = game_setting_definition::lookup("fDefaultRelaunchInterval")) {
         loaded_game_setting loaded;
         if (this->stub.get_owning_load_order().get_loaded_setting_by_name(*dfn, loaded)) {
            this->relaunch_interval = loaded.value.f;
         } else {
            this->relaunch_interval = dfn->default_value.f;
         }
      } else {
         this->relaunch_interval = 0.25F;
      }
      this->relaunch_interval = 0;
      this->speed = 10000;
      this->tracer_chance = 0;
      this->type = projectile_type::arrow;
   }
   void Projectile::setup(const file_load_order& lo) noexcept {
      if (const auto* dfn = game_setting_definition::lookup("fDefaultRelaunchInterval")) {
         loaded_game_setting loaded;
         if (lo.get_loaded_setting_by_name(*dfn, loaded)) {
            this->relaunch_interval = loaded.value.f;
         } else {
            this->relaunch_interval = dfn->default_value.f;
         }
      } else {
         this->relaunch_interval = 0.25F;
      }
   }
   void Projectile::_sever_outbound_references_impl(form_stub& other) noexcept {
      if (this->destruction_data.has_value())
         this->destruction_data.value().sever_outbound_references_to(other, *this);
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);

      this->collision_layer.clear_if(*this, other);
      this->decal.clear_if(*this, other);
      this->default_weapon_source.clear_if(*this, other);
      this->explosion.form.clear_if(*this, other);
      this->light.clear_if(*this, other);
      this->muzzle_flash.light.clear_if(*this, other);
      this->muzzle_flash.model.sever_outbound_references_to(other, *this);
      this->sounds.countdown.clear_if(*this, other);
      this->sounds.disarm.clear_if(*this, other);
      this->sounds.flyby.clear_if(*this, other);
   }
}