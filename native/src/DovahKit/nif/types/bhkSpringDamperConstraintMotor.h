#pragma once

namespace nifDK {
   class file_reader;

   struct bhkSpringDamperConstraintMotor {
      struct {
         float minimum = -1000000;
         float maximum =  1000000;
      } force;
      struct {
         float constant = 0;
         float damping  = 0;
      } spring;
      bool  enabled = false;

      void read(file_reader&);
   };
}