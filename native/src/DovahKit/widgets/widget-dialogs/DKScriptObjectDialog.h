#pragma once
#include "ui_DKScriptObjectDialog.h"
#include <optional>
#include <QDialog>
#include "dovah/form_types.h"
#include "../widget-models/bound-scripts/property_value.h"
#include "../widget-models/DKBoundScriptModel.h"

class DKScriptObjectDialog : public QDialog {
   Q_OBJECT;
   public:
      DKScriptObjectDialog(QWidget&, QModelIndex scriptModelIndex);

   protected:
      Ui::DKScriptObjectDialog ui;
      QPersistentModelIndex    script_qmi;
      DKBoundScriptModel*      script_model = nullptr;

   protected:
      std::optional<size_t> _currentArrayElementIndex() const;
      QModelIndex _selectedPropertyQMI() const;

      std::optional<DKBoundScriptModel::PropertyInfo> _property_info(const QModelIndex&) const;
      std::optional<DKBoundScriptModel::PropertyInfo> _selected_property_info() const;
      ui::bound_script_models::vmad::property_type _selected_property_element_type() const;
      
   protected: // functions for editing the value
      void _move_currently_focused_array_element(bool move_down);
      void _set_currently_focused_value(const ui::bound_script_models::property_value&);

   protected: // functions for updating UI state
      void _showSelectedProperty();

      void _clear_displayed_typename();
      void _update_displayed_typename(QString);

      void _update_autofill_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);
      void _update_clear_edit_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);
      void _update_revert_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);

      void _populate_array_table(const ui::bound_script_models::property_value&);

      void _update_edit_widget_constraints(const DKBoundScriptModel::PropertyInfo&);

      // Make sure that any widget constraints (e.g. required scriptnames; required form type) are 
      // up to date before calling this, or the set operation may fail silently (i.e. because the 
      // value being set doesn't satisfy the constraints).
      void _populate_edit_widgets(const ui::bound_script_models::property_value&);

      void _show_edit_widgets(const std::optional<DKBoundScriptModel::PropertyInfo>&);
};