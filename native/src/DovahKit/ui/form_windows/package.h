#pragma once
#include "./_base.h"
#include "dovah/forms/Package.h"
#include "ui_package.h"

namespace dovah::loaded_forms::structs::typed_package_info {
   class custom;
}
class PackageDataModel;
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
         PackageTemplatePickerFilter* package_template = nullptr;
      } _filters;
      struct {
         PackageDataModel*          package_data = nullptr;
         PackageProcedureTreeModel* procedure_tree = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      custom_package_data* _get_custom_package_data();
      custom_package_data* _get_template_package_data();
      void _set_is_package_template(bool);

      void _on_packdata_selection_changed();
      void _on_packdata_declaration_edited();
      void _on_packdata_type_edited();
      void _on_packdata_value_edited();

      void _pull_procedure_node_to_ui();
      void _push_procedure_node_from_ui(QModelIndex dst = {}); // invalid index = "use selection"
      void _push_procedure_flag_overrides_from_ui(QModelIndex dst = {});
      void _on_procedure_tree_selection_changed(const QItemSelection& selected, const QItemSelection& deselected);
};
