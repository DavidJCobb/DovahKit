#include "./actor_value_index.h"
#include "dovah/forms/ActorValueInfo.h"
#include "editor/core.h"
#include "editor/subsystems/game_localized_strings/core.h"

namespace ui::enum_dropdown_configs {
   namespace impl {
      extern QString actor_value_form_name(const dovah::actor_value_info& av_info) {
         auto* av_stub = DovahKitCore::get().get_form_of_probable_type(dovah::form_type::actor_value_info, av_info.formID);
         if (!av_stub)
            return {};
         auto form_ptr = av_stub->load().ptr_cast<dovah::loaded_forms::ActorValueInfo>();
         if (!form_ptr)
            return {};
         auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
         return gls.convert_localized_string(form_ptr->name);
      }
   }
}