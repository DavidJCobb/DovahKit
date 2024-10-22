#include "./alias_subdialog_helpers.h"
#include "dovah/forms/Quest.h"
#include "editor/helpers/story_event_name.h"

namespace {
   using loaded_alias_type = dovah::loaded_forms::Alias;

   constexpr const auto no_alias = loaded_alias_type::none_id;

   using alias_type = loaded_alias_type::alias_type;
}

extern void make_location_alias_combobox(dovah::loaded_forms::Quest& quest, QComboBox& widget) {
   widget.clear();
   widget.addItem(QObject::tr("NONE", "alias name"), -1);
   for (auto* alias : quest.aliases) {
      if (alias->type != alias_type::location)
         continue;
      widget.addItem(QString::fromStdString(alias->name), alias->id);
   }
}
extern void make_reference_alias_combobox(dovah::loaded_forms::Quest& quest, QComboBox& widget) {
   widget.clear();
   widget.addItem(QObject::tr("NONE", "alias name"), -1);
   for (auto* alias : quest.aliases) {
      if (alias->type != alias_type::reference)
         continue;
      widget.addItem(QString::fromStdString(alias->name), alias->id);
   }
}
extern void set_combobox_to_alias(QComboBox& widget, uint32_t alias_id) {
   int i = widget.findData(alias_id == no_alias ? -1 : alias_id);
   if (i < 0)
      i = 0;
   widget.setCurrentIndex(i);
}

extern void make_event_data_comboboxes(dovah::loaded_forms::Quest& quest, QComboBox& event_code, QComboBox& event_data) {
   event_code.clear();
   for (auto code : dovah::all_story_event_codes) {
      if (code == dovah::story_event_code::none)
         continue;
      if (code == dovah::story_event_code::undefined)
         continue;
      event_code.addItem(editor_helpers::story_event_name(code), (int)code);
   }
   event_code.model()->sort(0, Qt::SortOrder::AscendingOrder);
   event_code.insertItem(0, editor_helpers::story_event_name(dovah::story_event_code::none), (int)dovah::story_event_code::none);
   event_code.setCurrentIndex(0);

   QObject::connect(&event_code, QOverload<int>::of(&QComboBox::currentIndexChanged), &event_data, [&event_code, &event_data]() {
      auto event = (dovah::story_event_code::type)event_code.currentData().toInt();
      if (event == dovah::story_event_code::none) {
         event_data.clear();
         event_data.setEnabled(false);
         return;
      }
      const auto* dfn = dovah::story_event_definition::lookup(event);
      if (!dfn) {
         event_data.clear();
         event_data.setEnabled(false);
         return;
      }
      event_data.clear();
      for (const auto& member : dfn->members) {
         bool valid = false;
         for (auto ft : member.allowed_form_types) {
            if (ft == dovah::form_type::location) {
               valid = true;
               break;
            }
         }
         if (!valid)
            continue;

         event_data.addItem(member.name, member.signature);
      }
      if (event_data.count() > 0) {
         event_data.model()->sort(0, Qt::SortOrder::AscendingOrder);
         event_data.insertItem(0, editor_helpers::story_event_name(dovah::story_event_code::none), 0);
         event_data.setEnabled(true);
      } else {
         event_data.setEnabled(false);
      }
   });

   int i = event_code.findData(quest.event);
   if (i < 0)
      i = 0;
   event_code.setCurrentIndex(i);
   event_code.setEnabled(false);
}