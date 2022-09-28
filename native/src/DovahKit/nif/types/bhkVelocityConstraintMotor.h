#pragma once

namespace nifDK {
   class file_reader;

   struct bhkVelocityConstraintMotor {
      struct {
         float minimum = -1000000;
         float maximum =  1000000;
      } force;
      float tau                 = 0.8F; // relative stiffness
      float target_velocity     = 0.0F;
      bool  use_target_velocity = false;
      bool  enabled             = false;

      void read(file_reader&);
   };
}