#include "wrapper.h"
#include <cassert>
#include "editor_script_core.h"
#include "util.h"

namespace editor_script {
   bool wrapper::is_equal(const wrapper* other) const noexcept {
      if (this->type != other->type)
         return false;
      if (this->type == wrapper_type::form_data) {
         if (this->stub != other->stub)
            return false;
      }
      if (typeid(this) != typeid(other))
         return false;
      return this->_is_equal_impl(other);
   }
}