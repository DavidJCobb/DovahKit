#pragma once

namespace nifDK {
   class file_reader;

   struct bhkPositionConstraintMotor {
      struct {
         float minimum = -1000000;
         float maximum =  1000000;
      } force;
      float tau     = 0.8F; // relative stiffness
      float damping = 1.0F;
      struct {
         float proportional = 2.0F;
         float constant     = 1.0F;
      } recovery_velocity;
      bool enabled = false;

      void read(file_reader&);
   };
}