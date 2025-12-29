#include "./auto_door_envelopes_teleport_marker.h"
#include "../data/game_settings.h"
#include "../forms/components/extra_data/types/t/teleport.h"
#include "../form_stubs/helpers/get_base_form.h"
#include "../forms/Door.h"
#include "../forms/ObjectReference.h"

namespace dovah::utils {
   extern bool auto_door_envelopes_teleport_marker(loaded_forms::ObjectReference& ref) {
      auto* extra_teleport = ref.extra_data.get<loaded_forms::components::extra_data_types::teleport>();
      if (!extra_teleport)
         return false;

      form_stub* dst_ref  = extra_teleport->target_door.get_form_stub();
      form_stub* dst_base_stub = nullptr;
      if (!dst_ref || !form_type_is_reference(dst_ref->form_type))
         return false;
      auto dst_form = dst_ref->load().ptr_cast<loaded_forms::ObjectReference>();
      if (!dst_form)
         return false;
      
      dst_base_stub = dst_form->base_form.get_form_stub();
      if (!dst_base_stub || dst_base_stub->form_type != dovah::form_type::door)
         return false;

      auto dst_base_form = dst_base_stub->load().ptr_cast<loaded_forms::Door>();
      if (!dst_base_form)
         return false;
      if (!(dst_base_form->door_flags & loaded_forms::Door::door_flag::automatic))
         return false;

      const auto& game_setting = []() -> const auto& {
         constexpr const auto name = std::string_view("fAutoDoorActivateDistance");
         for (auto& item : game_settings)
            if (item.name == name)
               return item;
         std::unreachable();
      }();
      float auto_door_radius = game_setting.default_value.f;
      {
         auto& lo = ref.stub.get_owning_load_order();
         loaded_game_setting loaded;
         if (lo.get_loaded_setting_by_name("fAutoDoorActivateDistance", loaded)) {
            auto_door_radius = loaded.value.f;
         }
      }

      auto gap = extra_teleport->position - dst_form->position;
      return (gap.length() <= auto_door_radius);
   }
   extern bool auto_door_envelopes_teleport_marker(form_stub& ref) {
      if (!form_type_is_reference(ref.form_type))
         return false;
      auto loaded = ref.load().ptr_cast<loaded_forms::ObjectReference>();
      if (!loaded)
         return false;
      return auto_door_envelopes_teleport_marker(*loaded);
   }
}