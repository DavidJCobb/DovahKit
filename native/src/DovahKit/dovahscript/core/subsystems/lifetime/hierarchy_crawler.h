#pragma once
#include <cstdint>
#include <type_traits>
#include <QObject>
#include "task_reference_state_multi_checker.h"

class  DovahscriptStandardItemModel;
struct ObservableStandardItemModelObserver;
namespace dovahscript::core::subsystems {
   class lifetime;
   class userdata;
}

namespace dovahscript::impl {
   class hierarchy_crawler {
      public:
         struct hierarchy_flag {
            enum type : uint8_t {
               referenced_in_lua  = 0x01,
               referenced_in_task = 0x02,
               referenced_general = 0x04, // we're sure the item is referenced, but we're not 100% sure where anymore
               //
               referenced_across_bridge = 0x40,
               marked_for_delete        = 0x80,
               //
               referenced_anywhere = referenced_in_lua | referenced_in_task | referenced_general | referenced_across_bridge,
            };
         };
         using hierarchy_flags_t = std::underlying_type_t<hierarchy_flag::type>;

         struct found_hierarchy {
            QObject* root = nullptr;
            QVector<QObject*> unowned_bridges; // bridge objects that aren't owned by any hierarchy item, e.g. QButtonGroups
            QVector<found_hierarchy*> bridged_to;
            hierarchy_flags_t flags        = 0;
            unsigned int      widget_count = 0; // only valid for hierarchies that weren't referenced except possibly across a bridge
         };

      protected:
         // This will be the UI parent of the scripted UI -- that is, the hardcoded UI which serves as 
         // the parent window to all scripted UI windows.
         QWidget* stop_at = nullptr;

         core::subsystems::lifetime& lifetime_sys;
         core::subsystems::userdata& userdata_sys;
         task_reference_state_multi_checker task_ref_checker;

      public:
         QVector<DovahscriptStandardItemModel*> models_known_to_be_referenced;
         QVector<found_hierarchy*> found;
         struct {
            QVector<QObject*> objects;
            QVector<ObservableStandardItemModelObserver*> model_observers;
         } severed_bridges;
         int total_widgets_deleted = 0;

         hierarchy_crawler();
         ~hierarchy_crawler();

         void crawl_from(QObject&);
         void crawl_from(ObservableStandardItemModelObserver&);
         void finalize();
         void delete_abandoned();
   };
}
