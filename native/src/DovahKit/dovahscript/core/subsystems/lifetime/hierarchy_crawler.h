#pragma once
#include <cstdint>
#include <type_traits>
#include <QObject>

class DovahscriptStandardItemModel;
class ObservableStandardItemModelObserver;
namespace dovahscript::core::subsystems {
   class lifetime;
}

namespace dovahscript::impl {
   class hierarchy_crawler {
      public:
         struct hierarchy_flag {
            enum type : uint8_t {
               referenced_in_lua  = 0x01,
               referenced_in_task = 0x02,
               //
               referenced_across_bridge = 0x40,
               marked_for_delete        = 0x80,
               //
               referenced_anywhere = referenced_in_lua | referenced_in_task | referenced_across_bridge,
            };
         };
         using hierarchy_flags_t = std::underlying_type_t<hierarchy_flag::type>;

         struct found_hierarchy {
            QObject* root = nullptr;
            QVector<QObject*> unowned_bridges; // bridge objects that aren't owned by any hierarchy item, e.g. QButtonGroups
            QVector<found_hierarchy*> bridged_to;
            hierarchy_flags_t flags = 0;
            unsigned int widget_count = 0;
         };

      protected:
         // This will be the UI parent of the scripted UI -- that is, the hardcoded UI which serves as 
         // the parent window to all scripted UI windows.
         QWidget* stop_at = nullptr;

         core::subsystems::lifetime& lifetime_sys;

      public:
         QVector<DovahscriptStandardItemModel*> models_known_to_be_referenced;
         QVector<found_hierarchy*> found;
         QVector<QObject*> severed_bridges;
         int total_widgets_deleted = 0;

         hierarchy_crawler();
         ~hierarchy_crawler();

         void crawl_from(QObject&);
         void crawl_from(ObservableStandardItemModelObserver&);
         void finalize();
         void delete_abandoned();
   };
}
