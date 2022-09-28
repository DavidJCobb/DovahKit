#include "bhkRigidBody.h"
#include "../reader.h"

namespace nifDK::block_types {
   void bhkRigidBody::collision_response::read(file_reader& reader) {
      reader.read(this->type);
      reader.read(this->pad01);
      reader.read(this->process_contact_callback_delay);
   }

   void bhkRigidBody::parse(file_reader& reader) {
      bhkEntity::parse(reader);

      reader.read(this->response);
      if (reader.version() >= file_version::from_parts<10, 1, 0, 0>) {
         reader.read(this->unk_int_1);
         reader.read(this->filter);
         reader.read(this->pad0C);
         if (reader.user_version<2>() > 34) {
            reader.read(this->unk_int_2);
         }
         reader.read(this->response_alt);
         if (reader.user_version<2>() > 34) {
            reader.read(this->unk_int_3);
         }
      }
      reader.read(this->translation);
      reader.read(this->rotation);
      reader.read(this->velocity.linear);
      reader.read(this->velocity.angular);
      reader.read(this->inertia_tensor);
      reader.read(this->center_of_mass);
      reader.read(this->mass);
      reader.read(this->damping.linear);
      reader.read(this->damping.angular);
      if (reader.user_version<2>() > 34) {
         reader.read(this->time_factor);
         if (reader.user_version<2>() != 130) {
            reader.read(this->gravity_factor);
         }
      }
      reader.read(this->friction);
      if (reader.user_version<2>() > 34) {
         reader.read(this->rolling_friction_mult);
      }
      reader.read(this->restitution);
      if (reader.version() >= file_version::from_parts<10, 1, 0, 0>) {
         reader.read(this->max_velocity.linear);
         reader.read(this->max_velocity.angular);
         if (reader.user_version<2>() != 130) {
            reader.read(this->penetration_depth);
         }
      }
      reader.read(this->motion_type);
      if (reader.user_version<2>() <= 34) {
         reader.read(this->deactivation.type);
      } else {
         reader.read(this->deactivation.enabled);
      }
      reader.read(this->deactivation.solver);
      reader.read(this->quality);
      if (reader.user_version<2>() == 130) {
         reader.read(this->penetration_depth);
         reader.read(this->unknown_float);
      }
      reader.read(this->unknown_bytes_x12);
      if (reader.user_version<2>() > 34) {
         reader.read(this->unknown_bytes_x4);
      }

      uint32_t count;
      reader.read(count);
      this->constraints.resize(count);
      for (auto& item : this->constraints)
         reader.read_ref(item);

      if (reader.user_version<2>() < 76) {
         uint32_t flags;
         reader.read(flags);
         this->body_flags = flags;
      } else {
         uint16_t flags;
         reader.read(flags);
         this->body_flags = flags;
      }
   }
}