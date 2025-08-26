#pragma once
#include <optional>
#include <QMenu>
#include "./_base.h"
#include "dovah/forms/Package.h"
#include "ui_package.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class custom;
}
class PackageDataModel;
class PackageProcedureParamsModel;
class PackageProcedureTreeModel;
class PackageTemplatePickerFilter;

class FormDialogPackage :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Package, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      using custom_package_data = dovah::loaded_forms::structs::typed_package_info::custom;

   public:
      FormDialogPackage(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogPackage ui;
      struct {
         struct {
            QMenu    menu;
            QAction* create_branch    = nullptr;
            QAction* create_procedure = nullptr;
            QAction* remove = nullptr;
         } procedure_tree;
      } _context_menus;
      struct {
         PackageTemplatePickerFilter* package_template = nullptr;
      } _filters;
      struct {
         PackageDataModel*            package_data = nullptr;
         PackageProcedureTreeModel*   procedure_tree = nullptr;
         PackageProcedureParamsModel* procedure_params = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      custom_package_data* _get_custom_package_data();
      custom_package_data* _get_template_package_data();
      void _set_is_package_template(bool);

      std::optional<size_t> _selected_packdata_row() const;
      void _on_packdata_selection_changed();
      void _on_packdata_declaration_edited();
      void _on_packdata_type_edited();
      void _on_packdata_value_edited();

      QModelIndex _selected_procedure_node_qmi() const;
      void _pull_procedure_node_to_ui();
      void _push_procedure_node_from_ui(QModelIndex dst = {}); // invalid index = "use selection"
      void _push_procedure_flag_overrides_from_ui(QModelIndex dst = {});
      void _on_procedure_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);

      void _update_procedure_params_picker();
      void _update_procedure_params_list(QModelIndex dst = {});
      void _update_packdata_deleteable() const;
};
