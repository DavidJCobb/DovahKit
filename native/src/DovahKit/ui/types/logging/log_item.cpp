#include "./log_item.h"
#include "editor/helpers/backend_error_to_string.h"
#include "editor/helpers/backend_warning_to_string.h"

#include "dovah/notices/base_file_load_error.h"
#include "dovah/notices/base_file_load_warning.h"
#include "dovah/notices/base_form_load_warning.h"
#include "dovah/notices/base_form_save_error.h"
#include "dovah/notices/base_form_save_warning.h"

namespace ui::types {
   log_item::log_item(
      const QString&   text,
      log_item_type    type,
      log_item_context context
   ) : text(text), type(type), context(context) {
   }
   log_item::log_item(const dovah::notices::base_error& notice) {
      this->type = log_item_type::error;
      this->text = editor_helpers::backend_error_to_string(notice);

      if (auto* casted = dynamic_cast<const dovah::notices::base_form_save_error*>(&notice)) {
         this->context = log_item_context::form_save;
      } else if (auto* casted = dynamic_cast<const dovah::notices::base_file_load_error*>(&notice)) {
         this->context = log_item_context::file_load;
         this->file    = QString::fromUtf8(QByteArray::fromStdString(casted->filename));
      }
   }
   log_item::log_item(const dovah::notices::base_warning& notice) {
      this->type = log_item_type::warning;
      this->text = editor_helpers::backend_warning_to_string(notice);

      if (auto* casted = dynamic_cast<const dovah::notices::base_form_load_warning*>(&notice)) {
         this->context = log_item_context::form_load;
         this->file    = QString::fromUtf8(QByteArray::fromStdString(casted->record_info.source_file));
      } else if (auto* casted = dynamic_cast<const dovah::notices::base_form_save_warning*>(&notice)) {
         this->context = log_item_context::form_save;
      } else if (auto* casted = dynamic_cast<const dovah::notices::base_file_load_warning*>(&notice)) {
         this->context = log_item_context::file_load;
         this->file    = QString::fromUtf8(QByteArray::fromStdString(casted->source_file));
      }
   }

   bool log_item::empty() const noexcept {
      return this->text.isEmpty();
   }
}