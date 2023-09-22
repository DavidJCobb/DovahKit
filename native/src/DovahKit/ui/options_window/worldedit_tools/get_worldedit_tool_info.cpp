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

/*static*/ worldedit_tool_info worldedit_tool_info::get_single_info_sans_id(dovahkit::subsystems::worldedit::tools::tool_id id) {
   using namespace dovahkit::subsystems::worldedit::tools;

   switch (id) {
      case id_of_none:
         return {
            .name = QObject::tr("<None>", "Worldedit tool name"),
         };
      case id_of<attempt_on_screen_selection>:
         return {
            .name = QObject::tr("Selection: Modify", "Worldedit tool name"),
         };
      case id_of<modify_camera_speed_flags>:
         return {
            .name        = QObject::tr("Camera: Change Speed Flags", "Worldedit tool name"),
            .description = QObject::tr("Set whether the camera is boosting at high speed, or moving at a low speed suitable for precision editing.", "Worldedit tool description"),
         };
      case id_of<move_camera>:
         return {
            .name = QObject::tr("Camera: Move", "Worldedit tool name"),
         };
      case id_of<move_selection>:
         return {
            .name = QObject::tr("Selection: Move (Translate)", "Worldedit tool name"),
         };
      case id_of< set_edit_gizmo_mode>:
         return {
            .name        = QObject::tr("Edit Gizmo: Change Mode and/or Reference Frame", "Worldedit tool name"),
            .description = QObject::tr("Switch between the translate, rotate, and scale gizmos, and/or set the reference frame used for the gizmo axes.", "Worldedit tool description"),
         };
      case id_of<turn_camera>:
         return {
            .name = QObject::tr("Camera: Turn", "Worldedit tool name"),
         };
      #if _DEBUG
      case id_of<debug_dump_landscape_details>:
         return {
            .name        = QObject::tr("Debug: Dump Landscape Details", "Worldedit tool name"),
            .description = QObject::tr("Console-print information about the landscape at the raycast hit position.", "Worldedit tool description"),
         };
      case id_of<debug_dump_raycast>:
         return {
            .name        = QObject::tr("Debug: Dump Raycast Hit Info", "Worldedit tool name"),
            .description = QObject::tr("Console-print information about the raycast hit position.", "Worldedit tool description"),
         };
      case id_of<debug_print>:
         return {
            .name        = QObject::tr("Debug: Print", "Worldedit tool name"),
            .description = QObject::tr("Console-print a string. This exists to help DovahKit's developers; you probably shouldn't even be able to see it.", "Worldedit tool description"),
         };
      #endif
   }
   return {};
}