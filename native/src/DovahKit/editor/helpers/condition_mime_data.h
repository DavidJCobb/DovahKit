#pragma once
#include <vector>
#include <QDataStream>
#include <QMimeData>
#include "ui/types/conditions/condition.h"

namespace editor_helpers {
   constexpr const char* const form_condition_array_mime_type  = "application/dovah-kit.form-condition-array";

   extern std::vector<ui::types::conditions::condition> conditions_from_mime_data(const QMimeData&);

   extern void append_condition_to_mime_data_stream(QDataStream&, const ui::types::conditions::condition&);
   extern void pull_condition_from_mime_data_stream(QDataStream&, ui::types::conditions::condition&);

   extern void add_conditions_to_mime_data(QMimeData&, const std::vector<ui::types::conditions::condition>& list, const ui::types::conditions::context&);
}