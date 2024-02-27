#pragma once
#include <vector>
#include <QString>
#include <QVector>
#include "dovah/form_types.h"

namespace dovah {
   class form_stub;
}

namespace ui::object_window {
   enum class filter_type {
      model_path,
      quest_prefix,
   };

   struct filter_info {
      QVector<dovah::form_type_t> form_types;
      struct {
         QString model_path_prefix;
         QString quest_filter_prefix;
      } filters;

      bool empty() const noexcept;
   
      bool operator==(const filter_info& other) const noexcept;

      static QString normalize_pathlike_string(const QString& filter) noexcept;

      bool form_matches_filters(const dovah::form_stub&) const noexcept;
   };
}