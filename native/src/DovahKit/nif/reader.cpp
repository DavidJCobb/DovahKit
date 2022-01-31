#include "reader.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

namespace nifDK {
   bool file_reader::read(NiMatrix33& v) {
      if (!this->is_in_bounds(sizeof(float) * 9))
         return false;
      this->unchecked_read(v);
      return true;
   }
   void file_reader::unchecked_read(NiMatrix33& v) {
      // in order from top left to bottom left, then top middle to bottom middle, then top right to bottom right
      this->unchecked_read(v[0][0]);
      this->unchecked_read(v[0][1]);
      this->unchecked_read(v[0][2]);
      this->unchecked_read(v[1][0]);
      this->unchecked_read(v[1][1]);
      this->unchecked_read(v[1][2]);
      this->unchecked_read(v[2][0]);
      this->unchecked_read(v[2][1]);
      this->unchecked_read(v[2][2]);
   }
   bool file_reader::read(NiTransform& v) {
      if (!this->is_in_bounds(sizeof(float) * (9 + 3 + 1)))
         return false;
      this->unchecked_read(v);
      return true;
   }
   void file_reader::unchecked_read(NiTransform& v) {
      this->unchecked_read(v.rotation);
      this->unchecked_read(v.position[0]);
      this->unchecked_read(v.position[1]);
      this->unchecked_read(v.position[2]);
      this->unchecked_read(v.scale);
   }
}