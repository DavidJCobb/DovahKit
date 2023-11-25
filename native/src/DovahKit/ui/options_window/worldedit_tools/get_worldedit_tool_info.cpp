#include "./get_worldedit_tool_info.h"
#include <QObject>
#include "helpers/unreachable.h"
#include "editor/subsystems/worldedit/tool_system/tools/_all.h"
#include "editor/subsystems/worldedit/tool_system/id_of.h"

/*static*/ const QVector<worldedit_tool_info>& worldedit_tool_info::get_all_info() {
   static QVector<worldedit_tool_info> info;
   if (info.isEmpty()) {
      using namespace dovahkit::subsystems::worldedit::tools;

      for (size_t i = 0; i < all_tools::count; ++i) {
         info.push_back(get_single_info_sans_id(i));
         info.back().id = i;
      }
      std::stable_sort(
         info.begin(),
         info.end(),
         [](const worldedit_tool_info& a, const worldedit_tool_info& b) {
            return a.name.compare(b.name) < 0;
         }
      );
      info.push_front(get_single_info_sans_id(id_of_none));
      info.front().id = id_of_none;
   }
   return info;
}

/*static*/ const worldedit_tool_info* worldedit_tool_info::info_of(dovahkit::subsystems::worldedit::tools::tool_id id) {
   for (const auto& item : get_all_info())
      if (item.id == id)
         return &item;
   return nullptr;
}

namespace {
   struct bare_tool_ui_info {
      const char* name = nullptr;
      const char* desc = nullptr;
   };

   constexpr const auto all_bare_tool_ui_info = []() {
      constexpr const size_t count = dovahkit::subsystems::worldedit::tools::all_tools::count;

      std::array<bare_tool_ui_info, count> out = {};
      {
         using namespace dovahkit::subsystems::worldedit::tools;

         out[id_of<attempt_on_screen_selection>] = {
            .name = "Selection: Modify"
         };
         out[id_of<debug_dump_landscape_details>] = {
            .name = "Debug: Dump Landscape Details",
            .desc = "Console-print information about the landscape at the raycast hit position. This exists to help DovahKit's developers; you probably shouldn't even be able to see it."
         };
         out[id_of<debug_dump_raycast>] = {
            .name = "Debug: Dump Raycast Hit Info",
            .desc = "Console-print information about the raycast hit position. This exists to help DovahKit's developers; you probably shouldn't even be able to see it."
         };
         out[id_of<debug_print>] = {
            .name = "Debug: Print",
            .desc = "Console-print a string. This exists to help DovahKit's developers; you probably shouldn't even be able to see it."
         };
         out[id_of<modify_camera_speed_flags>] = {
            .name = "Camera: Change Speed Flags",
            .desc = "Set whether the camera is boosting at high speed, or moving at a low speed suitable for precision editing."
         };
         out[id_of<move_camera>] = {
            .name = "Camera: Move",
         };
         out[id_of<move_selection>] = {
            .name = "Selection: Move (Translate)"
         };
         out[id_of<move_selection_by_drag>] = {
            .name = "Selection: Move (Translate) By Drag",
            .desc = "Drag-move all selected entities along an axis or plane."
         };
         out[id_of<orbit_camera>] = {
            .name = "Camera: Orbit",
            .desc = "Rotate and move the camera to orbit it around the current selection. This is similar to \"arcball\" camera controls."
         };
         out[id_of<scale_selection>] = {
            .name = "Selection: Scale"
         };
         out[id_of<set_edit_gizmo_mode>] = {
            .name = "Edit Gizmo: Change Mode and/or Reference Frame",
            .desc = "Switch between the translate, rotate, and scale gizmos, and/or set the reference frame used for the gizmo axes."
         };
         out[id_of<turn_camera>] = {
            .name = "Camera: Turn",
         };
      }
      dovahkit::subsystems::worldedit::tools::all_tools::for_each([&out]<typename Current>() {
         auto id = dovahkit::subsystems::worldedit::tools::id_of<Current>;
         if (out[id].name == nullptr)
            out[id].name = Current::function_name;
      });
      return out;
   }();
}

/*static*/ worldedit_tool_info worldedit_tool_info::get_single_info_sans_id(dovahkit::subsystems::worldedit::tools::tool_id id) {
   using namespace dovahkit::subsystems::worldedit::tools;

   if (id == id_of_none) {
      return {
         .name = QObject::tr("<None>", "Worldedit tool name"),
      };
   }
   if (id < all_bare_tool_ui_info.size()) {
      return {
         .name        = QObject::tr(all_bare_tool_ui_info[id].name, "Worldedit tool name"),
         .description = QObject::tr(all_bare_tool_ui_info[id].desc, "Worldedit tool description"),
      };
   }
   return {
      .name = QString("!ERROR: Out-of-bounds tool index %1").arg(id),
   };
}