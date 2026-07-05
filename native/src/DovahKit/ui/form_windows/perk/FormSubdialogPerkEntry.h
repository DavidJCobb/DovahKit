#pragma once
#include <array>
#include <string>
#include <vector>
#include <QDialog>
#include <QPointer>
#include "ui_FormSubdialogPerkEntry.h" // generated
#include "ui/types/perk_entries/entry.h"
namespace dovah::loaded_forms {
   class Perk;
}
class DKPapyrusBoundScriptListPane;

class FormSubdialogPerkEntry : public QDialog {
   Q_OBJECT;
   public:
      using value_type = ui::types::perk_entries::entry;

   public:
      FormSubdialogPerkEntry(dovah::loaded_forms::Perk&, QWidget* parent = nullptr);

      void setScriptListWidget(DKPapyrusBoundScriptListPane*);

      value_type value() const;
      void setValue(const value_type&);
      
   protected:
      Ui::FormSubdialogPerkEntry ui;
      struct {
         dovah::loaded_forms::Perk& form;
         QPointer<DKPapyrusBoundScriptListPane> script_list_widget;
      } _state;

      void _update_options();
      void _rebuild_entry_point_condition_tabs(const dovah::perk_entry_point_info*);
      void _rebuild_entry_point_function_type_combobox(dovah::entry_point_value_type);
      void _update_entry_point_arguments(dovah::entry_point_function);
};