#pragma once
#include <QString>
#include "editor/subsystems/worldedit/tool_system/tool_id.h"

// Info for the UI.
struct worldedit_tool_info {
   dovahkit::subsystems::worldedit::tools::tool_id id;
   QString name;
   QString description;

   static const QVector<worldedit_tool_info>& get_all_info();
   private:
      static worldedit_tool_info get_single_info_sans_id(dovahkit::subsystems::worldedit::tools::tool_id);
};
