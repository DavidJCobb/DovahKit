#include "./dialogue.h"
#include <memory>
#include "../../_common_cpp.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   /*virtual*/ void dialogue::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      subrecord.read(this->fov);
      if (auto& form = this->topic; subrecord.read(form)) {
         intfc.warn_if_ref_is_wrong_type(form, form_type::topic, subrecord.signature());
      }
      subrecord.read(this->no_headtracking);
      subrecord.read(this->dont_control_target_movement);
      subrecord.skip_bytes(2);
      subrecord.read(this->unk0C);
      subrecord.read(this->dialogue_type);
      subrecord.read(this->unk14);
   }
   /*static*/ void dialogue::generate_header_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() != header_subrecord)
         return;

      form_id_t topic;

      subrecord.skip_bytes(sizeof(fov));
      subrecord.read(topic);
      if (topic)
         uib.add_outbound_reference(topic);
   }
   /*virtual*/ void dialogue::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(header_subrecord);
      subrecord.write(this->fov);
      subrecord.write(this->topic);
      subrecord.write(this->no_headtracking);
      subrecord.write(this->dont_control_target_movement);
      subrecord.skip_bytes(2);
      subrecord.write(this->unk0C);
      subrecord.write(this->dialogue_type);
      subrecord.write(this->unk14);
      subrecord.close();
      if (auto& opt = this->trigger_location; opt.has_value()) {
         auto& loc = opt.value();
         auto& subrecord = record.open_next_subrecord(package_location::subrecord_legacy_second);
         loc.save(subrecord, intfc);
         subrecord.close();
      }
   }
   /*virtual*/ base* dialogue::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  clone_ptr = std::make_unique<dialogue>();
      auto* clone     = clone_ptr.get();

      {
         auto& src = this->wait_location;
         auto& dst = clone->wait_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      {
         auto& src = this->trigger_location;
         auto& dst = clone->trigger_location;
         if (src.has_value())
            dst.emplace().clone_from(src.value(), owner_of_clone);
      }
      clone->target.clone_from(this->target, owner_of_clone);

      clone->fov = this->fov;
      clone->topic.set(owner_of_clone, this->topic);
      clone->no_headtracking = this->no_headtracking;
      clone->dont_control_target_movement = this->dont_control_target_movement;
      clone->unk0C = this->unk0C;
      clone->dialogue_type = this->dialogue_type;
      clone->unk14 = this->unk14;

      return clone_ptr.release();
   }
   /*virtual*/ void dialogue::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (auto& opt = this->wait_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      if (auto& opt = this->trigger_location; opt.has_value())
         opt.value().sever_outbound_references_to(other, my_owner);
      this->target.sever_outbound_references_to(other, my_owner);

      this->topic.clear_if(my_owner, other);
   }
   /*virtual*/ void dialogue::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (auto& opt = this->wait_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      if (auto& opt = this->trigger_location; opt.has_value()) {
         opt.value().clear(my_owner);
         opt.reset();
      }
      this->target.clear(my_owner);

      this->fov = 100;
      this->topic.set(my_owner, nullptr);
      this->no_headtracking = false;
      this->dont_control_target_movement = false;
      this->unk0C = 0;
      this->dialogue_type = dialogue_type_t::conversation;
      this->unk14 = 0;
   }
}