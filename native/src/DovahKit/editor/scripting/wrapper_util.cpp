#include "wrapper_util.h"
#include "wrappers/_all_forms.h"

namespace editor_script {
   extern bool part_type_uses_name_key(part_type_t signature) {
      switch (signature) {
      }
      return false;
   }

   extern const char* wrap_form(wrapper& out, dovah::form_stub* stub) {
      out.stub = stub;
      out.type = wrapper_type::form_data;
      switch (stub->formType) { // TODO: an actual list would maybe be more efficient than a switch-case once we end up with a large number of metatables here
         case dovah::form_type::voicetype:
            return wrappers::voicetype::metatable_key;
      }
      return wrappers::form::metatable_key;
   }
}