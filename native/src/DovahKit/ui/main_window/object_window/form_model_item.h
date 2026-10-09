#pragma once
#include <QAbstractItemModel>
#include <QString>
#include "dovah/bare_form_id_t.h"
namespace dovah {
   class form_stub;
}

namespace ui::object_window {
   class form_model_item {
      public:
         using form_id_type = dovah::bare_form_id_t;

      public:
         dovah::form_stub& stub;
         QString           editor_id;
         form_id_type      form_id    = 0;
         uint32_t          user_count = 0;
         
         bool is_active   = false;
         bool is_injected = false;
         bool is_none     = false;

      public:
         form_model_item(dovah::form_stub&);

         void update();
         bool update_user_count(); // returns true if any changes were made
   };

}