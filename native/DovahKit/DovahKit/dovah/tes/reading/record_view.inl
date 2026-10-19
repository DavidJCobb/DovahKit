#pragma once
#include "./record_view.h"

namespace dovah::tes {
   const subrecord_view& record_view::get_current_subrecord() const noexcept {
      return this->owner.subviews.subrecord;
   }
   subrecord_view& record_view::get_current_subrecord() noexcept {
      return this->owner.subviews.subrecord;
   }
}
