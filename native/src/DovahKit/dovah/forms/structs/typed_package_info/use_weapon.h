#pragma once
#include <optional>
#include "./base.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class use_weapon final : public base {
      public:
         static constexpr const legacy_type type = legacy_type::use_weapon;
         static constexpr const uint32_t    header_subrecord = 'PKW3';

         enum class fire_rate_type : uint8_t {
            auto_fire,
            volley_fire,
         };
         enum class fire_count_type : uint8_t {
            number_of_bursts,
            repeat_fire,
         };

      public:
         std::optional<package_location> use_at_location; // Location 1
         std::optional<package_location> target_location; // Location 2 // Wait until Target is within Target Location before attacking.
         package_target                legacy_weapon; // Target 1
         std::optional<package_target> target;        // Target 2 // CK UI code suggests that this is the ref to attack.

         struct {
            bool always_hit             = false; // PKW3+0x00
            bool do_no_damage           = false; // PKW3+0x01
            bool crouch_to_reload       = false; // PKW3+0x02
            bool hold_fire_when_blocked = false; // PKW3+0x04
            fire_rate_type  fire_rate   = fire_rate_type::auto_fire; // PKW3+0x04
            fire_count_type fire_count  = fire_count_type::number_of_bursts; // PKW3+0x05
            uint16_t        burst_count = 0; // PKW3+0x06
            struct {
               uint16_t min = 0; // PKW3+0x08
               uint16_t max = 0; // PKW3+0x0A
            } volley_shots;
            struct {
               float min = 0; // PKW3+0x0C
               float max = 0; // PKW3+0x10
            } volley_cooldown;
            form_reference_t weapon = 0; // PKW3+0x14 // WEAP form. Verified by RE.
         } header;

      public:
         virtual package_location* get_location_1() override { auto& o = this->use_at_location; return o.has_value() ? &o.value() : nullptr; };
         virtual package_location* get_location_2() override { auto& o = this->target_location; return o.has_value() ? &o.value() : nullptr; };
         virtual package_target* get_target_1() override { return &this->legacy_weapon; };
         virtual package_target* get_target_2() override { auto& o = this->target; return o.has_value() ? &o.value() : nullptr; };
         //
         virtual void set_location_1(const package_location& v) override { this->use_at_location = v; }
         virtual void set_location_2(const package_location& v) override { this->target_location = v; }
         virtual void set_target_1(const package_target& v) override { this->legacy_weapon = v; }
         virtual void set_target_2(const package_target& v) override { this->target = v; }
         //
         virtual bool is_of_legacy_type(legacy_type t) const noexcept override { return t == type; };

         virtual void load(tes_record_reader&, load_order_interfaces::form_load&) override;
         static void generate_header_use_info(tes_record_reader&, std::vector<form_id_t>& out);
         virtual void save(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual base* clone(loaded_forms::Form& owner_of_clone) const noexcept override;
         virtual void sever_outbound_references_to(form_stub&, loaded_forms::Form& my_containing_form) noexcept override;
         virtual void clear(loaded_forms::Form& my_containing_form) override;
   };
}