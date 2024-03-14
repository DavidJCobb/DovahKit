#pragma once
#include "ui_DKBoundScriptDialog.h"
#include <optional>
#include <QDialog>
#include "dovah/form_types.h"
#include "../widget-models/bound-scripts/property_value.h"
#include "../widget-models/DKBoundScriptModel.h"

class DKBoundScriptDialog : public QDialog {
   Q_OBJECT;
   public:
      DKBoundScriptDialog(QWidget&, QModelIndex scriptModelIndex);

   protected:
      Ui::DKBoundScriptDialog ui;
      QPersistentModelIndex    script_qmi;
      DKBoundScriptModel*      script_model = nullptr;

   public:
      bool loadFailed() const;

      virtual void showEvent(QShowEvent* event) override;

   protected:
      std::optional<size_t> _selected_array_element_index() const;
      QModelIndex _selected_property_qmi() const;

      std::optional<DKBoundScriptModel::PropertyInfo> _property_info(const QModelIndex&) const;
      std::optional<DKBoundScriptModel::PropertyInfo> _selected_property_info() const;
      ui::bound_script_models::vmad::property_type _selected_property_element_type() const;
      
   protected: // functions for editing the value
      void _move_currently_focused_array_element(bool move_down);
      void _set_currently_focused_value(const ui::bound_script_models::property_value&);

      void _set_selected_array_element_index(std::optional<size_t>);

   protected: // functions for updating UI state

      // Completely redo the entire array UI -- empty and refill the table, update all widgets, etc..
      void _refresh_array_ui(
         const ui::bound_script_models::property_value& array_value,
         std::optional<size_t> selected_element_index
      );

      // Completely redo the entire UI for selected properties -- element visibility, enable state, etc., for everything.
      void _refresh_property_ui();

      void _clear_displayed_typename();
      void _update_displayed_typename(QString);

      void _update_autofill_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);
      void _update_clear_edit_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);
      void _update_revert_button(const std::optional<DKBoundScriptModel::PropertyInfo>&);

      void _update_array_element_buttons();

      void _populate_array_table(const ui::bound_script_models::property_value&);

      void _clear_edit_widget_constraints();
      void _update_edit_widget_constraints(const DKBoundScriptModel::PropertyInfo&);

      // Make sure that any widget constraints (e.g. required scriptnames; required form type) are 
      // up to date before calling this, or the set operation may fail silently (i.e. because the 
      // value being set doesn't satisfy the constraints).
      void _populate_edit_widgets(const ui::bound_script_models::property_value&);

      void _populate_edit_widgets_for_array(const ui::bound_script_models::property_value&);

      void _show_edit_widgets(const std::optional<DKBoundScriptModel::PropertyInfo>&);
};