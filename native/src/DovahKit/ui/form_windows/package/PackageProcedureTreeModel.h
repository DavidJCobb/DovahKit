#pragma once
#include <cstdint>
#include <optional>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/data/packages/procedure_tree_branch_type.h"
#include "dovah/data/packages/procedure_type.h"
#include "dovah/forms/structs/custom_packages/package_flag_overrides.h"
#include "editor/q_declare_metatype/package_flag_overrides.h"
#include "ui/types/packages/procedure_tree_typed_data/branch.h"
#include "ui/types/packages/procedure_tree_typed_data/procedure.h"
#include "ui/types/packages/procedure_node.h"
namespace dovah {
   namespace loaded_forms {
      namespace structs::custom_packages {
         class procedure_tree;
      }
      class Form;
   }
   class form_stub;
}
class DKConditionList;

class PackageProcedureTreeModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      using backend_type = dovah::loaded_forms::structs::custom_packages::procedure_tree;
      using node_type    = ui::types::packages::procedure_node;

      using branch_type = dovah::packages::procedure_tree_branch_type;
      using branch_flag = ui::types::packages::procedure_tree_typed_data::branch::flag;

      using procedure_type = dovah::packages::procedure_type;
      using procedure_flag = ui::types::packages::procedure_tree_typed_data::procedure::flag;

      static constexpr const uint8_t no_unique_id = 0xFF;

   public:
      PackageProcedureTreeModel(QObject* parent = nullptr);

      static constexpr const size_t ColumnCount = 1;

      static constexpr const Qt::ItemDataRole BranchTypeRole     = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const Qt::ItemDataRole BranchFlagsRole    = (Qt::ItemDataRole)(Qt::UserRole + 2);
      static constexpr const Qt::ItemDataRole ProcedureTypeRole  = (Qt::ItemDataRole)(Qt::UserRole + 3);
      static constexpr const Qt::ItemDataRole ProcedureFlagsRole = (Qt::ItemDataRole)(Qt::UserRole + 4);
      static constexpr const Qt::ItemDataRole ProcedureOverrideFlagsRole = (Qt::ItemDataRole)(Qt::UserRole + 5); // dovah::loaded_forms::structs::custom_packages::package_flag_overrides
      
   protected:
      #pragma region Map QMIs to data
         QModelIndex _qmi_for_model() const; // i.e. the "invisible root item"
         QModelIndex _qmi_for_root() const;
         QModelIndex _qmi_for_orphan(size_t index) const;
         QModelIndex _qmi_for_child(const node_type&, int row, int col = 0) const;
         bool _is_model_qmi(const QModelIndex&) const;
         const node_type* _node_for_qmi(const QModelIndex&) const;
         node_type* _node_for_qmi(const QModelIndex& qmi) {
            return const_cast<node_type*>(std::as_const(*this)._node_for_qmi(qmi));
         }
      #pragma endregion

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            #pragma region Write-access
               virtual bool setData(const QModelIndex& index, const QVariant& value, int role) override;
            #pragma endregion
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      void clear();

      void import_tree(const backend_type& src);
      void export_tree(backend_type& dst, dovah::loaded_forms::Form& dst_containing_form) const;

      size_t procedureParameterIDCount(const QModelIndex&) const;
      uint8_t getProcedureParameterID(const QModelIndex&, size_t index) const;
      std::vector<uint8_t> getProcedureParameterIDs(const QModelIndex&, size_t index) const;
      void setProcedureParameterID(const QModelIndex&, size_t index, uint8_t unique_id);

      std::vector<ui::types::conditions::condition> nodeConditions(const QModelIndex&) const;
      void setNodeConditions(const QModelIndex&, const std::vector<ui::types::conditions::condition>& src);

   protected:
      std::unique_ptr<node_type>              _root;
      std::vector<std::unique_ptr<node_type>> _orphans;

      void _sever_uses_of_form(dovah::form_stub&);
};