#include "./use_weapon.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void use_weapon::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      subrecord.read(this->header.always_hit);
      subrecord.read(this->header.do_no_damage);
      subrecord.read(this->header.crouch_to_reload);
      subrecord.read(this->header.hold_fire_when_blocked);
      subrecord.read(this->header.fire_rate);
      subrecord.read(this->header.fire_count);
      subrecord.read(this->header.burst_count);
      subrecord.read(this->header.volley_shots.min);
      subrecord.read(this->header.volley_shots.max);
      subrecord.read(this->header.volley_cooldown.min);
      subrecord.read(this->header.volley_cooldown.max);
      if (auto& form = this->header.weapon; subrecord.read(form)) {
         intfc.warn_if_ref_is_wrong_type(form, form_type::weapon, subrecord.signature());
      }
   }
   /*static*/ void use_weapon::generate_header_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      form_id_t weapon;

      subrecord.skip_bytes(14);
      subrecord.read(weapon);
      if (weapon)
         uib.add_outbound_reference(weapon);
   }
   /*virtual*/ void use_weapon::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write(this->header.always_hit);
      subrecord.write(this->header.do_no_damage);
      subrecord.write(this->header.crouch_to_reload);
      subrecord.write(this->header.hold_fire_when_blocked);
      subrecord.write(this->header.fire_rate);
      subrecord.write(this->header.fire_count);
      subrecord.write(this->header.burst_count);
      subrecord.write(this->header.volley_shots.min);
      subrecord.write(this->header.volley_shots.max);
      subrecord.write(this->header.volley_cooldown.min);
      subrecord.write(this->header.volley_cooldown.max);
      subrecord.write(this->header.weapon);
      subrecord.close();
      if (auto& opt = this->target; opt.has_value()) {
         auto& tgt = opt.value();
         auto& subrecord = record.open_next_subrecord(package_target::subrecord_modern_second);
         tgt.save(subrecord, intfc);
         subrecord.close();
      }
      if (auto& opt = this->target_location; opt.has_value()) {
         auto& loc = opt.value();
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* use_weapon::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<use_weapon>();
      auto* clone     = clone_ptr.get();

      {
         auto& src = this->use_at_location;
         auto& dst = clone->use_at_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      {
         auto& src = this->target_location;
         auto& dst = clone->target_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->legacy_weapon.clone_from(this->legacy_weapon, owner_of_clone);
      {
         auto& src = this->target;
         auto& dst = clone->target;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }

      clone->header.always_hit = this->header.always_hit;
      clone->header.do_no_damage = this->header.do_no_damage;
      clone->header.crouch_to_reload = this->header.crouch_to_reload;
      clone->header.hold_fire_when_blocked = this->header.hold_fire_when_blocked;
      clone->header.fire_rate = this->header.fire_rate;
      clone->header.fire_count = this->header.fire_count;
      clone->header.burst_count = this->header.burst_count;
      clone->header.volley_shots = this->header.volley_shots;
      clone->header.volley_cooldown = this->header.volley_cooldown;
      clone->header.weapon.set(owner_of_clone, this->header.weapon);

      return clone_ptr.release();
   }
   /*virtual*/ void use_weapon::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (auto& opt = this->use_at_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->target_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->legacy_weapon.sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->target; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);

      this->header.weapon.clear_if(my_owner, other);
   }
   /*virtual*/ void use_weapon::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (auto& opt = this->use_at_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      if (auto& opt = this->target_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->legacy_weapon.clear(my_owner);
      if (auto& opt = this->target; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }

      this->header.always_hit = false;
      this->header.do_no_damage = false;
      this->header.crouch_to_reload = false;
      this->header.hold_fire_when_blocked = false;
      this->header.fire_rate = {};
      this->header.fire_count = {};
      this->header.burst_count = {};
      this->header.volley_shots = {};
      this->header.volley_cooldown = {};
      this->header.weapon.set(my_owner, nullptr);
   }
}