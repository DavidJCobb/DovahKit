#include "./music_track_type.h"
#include <QCoreApplication>
#include "dovah/data/music_track_type.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/music_track.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "widgets/DKFormListPane.h"

namespace ui::form_list_pane_columns {
   extern void music_track_type(DKFormListPane& widget) {
      widget.addExtraColumn("Type", [](const dovah::form_stub& stub) -> QString {
         if (stub.form_type != dovah::form_type::music_track)
            return "";
         auto& fic  = dovahkit::subsystems::form_info_cache::core::get();
         auto* info = fic.get_music_track_info(stub);
         if (!info)
            return "";
         switch (info->type) {
            case dovah::music_track_type::palette:
               return QCoreApplication::translate("dovah::music_track_type", "Palette");
            case dovah::music_track_type::silent:
               return QCoreApplication::translate("dovah::music_track_type", "Silent");
            case dovah::music_track_type::single:
               return QCoreApplication::translate("dovah::music_track_type", "Single");
         }
         return "";
      });
   }
}