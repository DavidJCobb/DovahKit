#pragma once
#include "helpers/function_pointer.h"
#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldedit/tool_system/tool_id.h"
#include "./options/_all.h"
#include "./all_tool_options_widgets.h"

class QWidget;
namespace dovahkit::ui::worldedit {
   namespace tools {
      class base;
   }
}

namespace dovahkit::ui::worldedit {
   constexpr const auto options_widget_dispatch_table = []() {
      using tool_id       = dovahkit::subsystems::worldedit::tools::tool_id;
      using options_union = dovahkit::subsystems::worldedit::tools::options_union;

      struct entry {
         tool_id id = dovahkit::subsystems::worldedit::tools::id_of_none;
         //
         cobb::function_pointer<tools::base*()> make_widget = nullptr;
         cobb::function_pointer<void(tools::base*, const options_union&)> to_ui = nullptr;
         cobb::function_pointer<void(tools::base*, options_union&)> to_data = nullptr;
      };

      std::array<entry, all_tool_options_widgets::count> table = {};

      size_t i = 0;
      all_tool_options_widgets::for_each([&table, &i]<typename Current>() {
         auto& item = table[i++];
         item.id          = dovahkit::subsystems::worldedit::tools::id_of<typename Current::tool>;
         item.make_widget = []() -> tools::base* { return new Current; };
         item.to_ui       = [](tools::base* widget, const options_union& ou) {
            ((Current*)widget)->set_options(ou.as<typename Current::options_type>());
         };
         item.to_data     = [](tools::base* widget, options_union& ou) {
            ou = options_union(((Current*)widget)->get_options());
         };
      });

      return table;
   }();
}