#include "form_info_cache.h"
#include "editor/subsystems/form_info_cache/form_info_cache.h"

namespace DovahKitDebug::features {
   /*static*/ void form_info_cache::execute(QWidget* window) {
      dovahkit::subsystems::form_info_cache::core::get_or_create();
      qDebug("Force-instantiated the form-info-cache.");
   }
}