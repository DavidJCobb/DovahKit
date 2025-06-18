#include "Ragdoll.h"
#include "_common_cpp.h"

namespace dovah::loaded_forms {
   void Ragdoll::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
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
            case 'MODL':
            case 'MODT':
               this->model.load(subrecord, intfc);
               break;
            case 'NVER':
               subrecord.read(this->version);
               break;
            case 'DATA':
               subrecord.read(this->data.dynamic_bone_count);
               subrecord.read(this->data.unk02);
               subrecord.read(this->data.unk04);
               subrecord.read(this->data.unk06);
               subrecord.read(this->data.feedback);
               subrecord.read(this->data.foot_ik);
               subrecord.read(this->data.look_ik);
               subrecord.read(this->data.grab_ik);
               subrecord.read(this->data.pose_matching);
               subrecord.read(this->data.unk0D);
               this->feedback_dynamic_bones.resize(this->data.dynamic_bone_count);
               break;
            case 'XNAM':
               if (auto& form = this->preview_actor; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::actor_base, subrecord.signature());
               break;
            case 'TNAM':
               if (auto& form = this->body_part_data; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::body_part_data, subrecord.signature());
               break;
            case 'RAFD':
               subrecord.read(this->feedback_data.dynamic_keyframe_blend_amount);
               subrecord.read(this->feedback_data.gain.hierarchy);
               subrecord.read(this->feedback_data.gain.position);
               subrecord.read(this->feedback_data.gain.velocity);
               subrecord.read(this->feedback_data.gain.acceleration);
               subrecord.read(this->feedback_data.gain.snap);
               subrecord.read(this->feedback_data.velocity_damping);
               subrecord.read(this->feedback_data.snap_max.velocity.linear);
               subrecord.read(this->feedback_data.snap_max.velocity.angular);
               subrecord.read(this->feedback_data.snap_max.distance.linear);
               subrecord.read(this->feedback_data.snap_max.distance.angular);
               subrecord.read(this->feedback_data.max_velocity.linear);
               subrecord.read(this->feedback_data.max_velocity.angular);
               subrecord.read(this->feedback_data.max_velocity.projectile);
               subrecord.read(this->feedback_data.max_velocity.melee);
               break;
            case 'RAFB': // present only if not empty
               //
               // We *should* probably warn if the size of this subrecord doesn't match the 
               // number of elements declared by DATA+0x00... but how would we even word a 
               // warning, were we to display it? "Hey, user, we have no idea what any of 
               // this is or does, but these fields, which you're equally clueless about, 
               // don't match."
               //
               for (auto& v : this->feedback_dynamic_bones)
                  subrecord.read(v);
               break;
            case 'RAPS':
               subrecord.read(this->pose_matching_data.bones);
               subrecord.read(this->pose_matching_data.disable_on_move);
               subrecord.read(this->pose_matching_data.motors_strength);
               subrecord.read(this->pose_matching_data.pose_activation_delay_time);
               subrecord.read(this->pose_matching_data.match_error_allowance);
               subrecord.read(this->pose_matching_data.displacement_to_disable);
               break;
            case 'ANAM':
               subrecord.read(this->death_pose);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Ragdoll::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files. (TODO: CONFIRM THIS)
         //
         return;

      form_id_t body_part_data;
      form_id_t preview_actor;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'TNAM':
               subrecord.read(body_part_data);
               break;
            case 'XNAM':
               subrecord.read(preview_actor);
               break;
         }
      }
      uib.add_outbound_reference(body_part_data);
      uib.add_outbound_reference(preview_actor);
   }
   void Ragdoll::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Ragdoll*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->death_pose = this->death_pose;
      copy->data = this->data;
      copy->feedback_dynamic_bones = this->feedback_dynamic_bones;
      copy->feedback_data = this->feedback_data;
      copy->pose_matching_data = this->pose_matching_data;

      copy->body_part_data.set(*copy, this->body_part_data);
      copy->preview_actor.set(*copy, this->preview_actor);
   }
   void Ragdoll::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      this->model.save(record, intfc, 'MODL', 'MODT');
      {
         auto& subrecord = record.open_next_subrecord('NVER');
         subrecord.write(this->version);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data.dynamic_bone_count);
         subrecord.write(this->data.unk02);
         subrecord.write(this->data.unk04);
         subrecord.write(this->data.unk06);
         subrecord.write(this->data.feedback);
         subrecord.write(this->data.foot_ik);
         subrecord.write(this->data.look_ik);
         subrecord.write(this->data.grab_ik);
         subrecord.write(this->data.pose_matching);
         subrecord.write(this->data.unk0D);
         subrecord.close();
      }
      record.write_formID_subrecord('XNAM', this->preview_actor, true);
      record.write_formID_subrecord('TNAM', this->body_part_data, true);
      {
         auto& subrecord = record.open_next_subrecord('RAFD');
         subrecord.write(this->feedback_data.dynamic_keyframe_blend_amount);
         subrecord.write(this->feedback_data.gain.hierarchy);
         subrecord.write(this->feedback_data.gain.position);
         subrecord.write(this->feedback_data.gain.velocity);
         subrecord.write(this->feedback_data.gain.acceleration);
         subrecord.write(this->feedback_data.gain.snap);
         subrecord.write(this->feedback_data.velocity_damping);
         subrecord.write(this->feedback_data.snap_max.velocity.linear);
         subrecord.write(this->feedback_data.snap_max.velocity.angular);
         subrecord.write(this->feedback_data.snap_max.distance.linear);
         subrecord.write(this->feedback_data.snap_max.distance.angular);
         subrecord.write(this->feedback_data.max_velocity.linear);
         subrecord.write(this->feedback_data.max_velocity.angular);
         subrecord.write(this->feedback_data.max_velocity.projectile);
         subrecord.write(this->feedback_data.max_velocity.melee);
         subrecord.close();
      }
      if (!this->feedback_dynamic_bones.empty()) {
         auto& subrecord = record.open_next_subrecord('RAFB');
         for (auto& v : this->feedback_dynamic_bones)
            subrecord.write(v);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('RAPS');
         subrecord.write(this->pose_matching_data.bones);
         subrecord.write(this->pose_matching_data.disable_on_move);
         subrecord.write(this->pose_matching_data.motors_strength);
         subrecord.write(this->pose_matching_data.pose_activation_delay_time);
         subrecord.write(this->pose_matching_data.match_error_allowance);
         subrecord.write(this->pose_matching_data.displacement_to_disable);
         subrecord.close();
      }
      record.write_string_subrecord('ANAM', this->death_pose);
   }
   void Ragdoll::_clear_impl() noexcept {
      this->model.clear();
      this->script_data.clear(*this);

      this->death_pose.clear();
      this->data = {};
      this->feedback_dynamic_bones.clear();
      this->feedback_data = {};
      this->pose_matching_data = {};

      this->body_part_data.set(*this, nullptr);
      this->preview_actor.set(*this, nullptr);
   }
   void Ragdoll::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->model.sever_outbound_references_to(other, *this);
      this->script_data.sever_outbound_references_to(other, *this);
      this->body_part_data.clear_if(*this, other);
      this->preview_actor.clear_if(*this, other);
   }
}