#include "./PackageProcedureTreeModel.h"
#include "helpers/bitset.h"
#include "helpers/vectors/move_item_within.h"
#include "dovah/forms/structs/custom_packages/procedure_tree.h"
#include "editor/core.h"
#include "editor/localize/package_procedure_tree_branch_type.h"
#include "editor/localize/package_procedure_type.h"

PackageProcedureTreeModel::PackageProcedureTreeModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, &PackageProcedureTreeModel::clear);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      assert(stub != nullptr);
      this->_sever_uses_of_form(*stub);
   });
}

#pragma region Map QMIs to data
   QModelIndex PackageProcedureTreeModel::_qmi_for_model() const {
      return this->createIndex(-1, -1, nullptr);
   }
   QModelIndex PackageProcedureTreeModel::_qmi_for_root() const {
      return this->createIndex(0, 0, (void*)this);
   }
   QModelIndex PackageProcedureTreeModel::_qmi_for_orphan(size_t index) const {
      return this->createIndex(index + 1, 0, (void*)this);
   }
   QModelIndex PackageProcedureTreeModel::_qmi_for_child(const node_type& node, int row, int col) const {
      return this->createIndex(row, col, (void*)&node);
   }
   bool PackageProcedureTreeModel::_is_model_qmi(const QModelIndex& qmi) const {
      return qmi.model() == this && !qmi.isValid();
   }
   const PackageProcedureTreeModel::node_type* PackageProcedureTreeModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (qmi.model() != this)
         return nullptr;
      auto row = qmi.row();
      if (row < 0)
         return nullptr;
      if (qmi.internalPointer() == this) {
         if (row == 0)
            return this->_root.get();
         --row;
         if (row >= this->_orphans.size())
            return nullptr;
         return this->_orphans[row].get();
      }
      if (const auto* parent = (node_type*)qmi.internalPointer()) {
         if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&parent->data)) {
            if (row >= casted->children.size())
               return nullptr;
            return casted->children[row].get();
         }
         return nullptr;
      }
      return nullptr;
   }
#pragma endregion
   
#pragma region QAbstractItemModel overrides
   #pragma region Hierarchy
      /*virtual*/ QModelIndex PackageProcedureTreeModel::index(int row, int col, const QModelIndex& parent) const /*override*/ {
         //
         // Get the index of child (row, col) in the given parent.
         //
         if (row < 0 || col < 0 || col >= this->columnCount())
            return {};
         auto* parent_node = _node_for_qmi(parent);
         if (!parent_node) {
            //
            // Return the QMI for a top-level node, i.e. the root node or an orphan.
            // 
            // The use of `>` rather than `>=` here is intentional, since for QMIs, 
            // row 0 is the root, and rows [1, n] are the orphans; ergo the "end" 
            // row index (i.e. one past the last) is `1 + this->_orphans.size()`.
            //
            if (row > this->_orphans.size())
               return this->createIndex(row, col, (void*)this);
         }
         if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&parent_node->data)) {
            if (row >= casted->children.size())
               return {};
            return this->createIndex(row, col, (void*)parent_node);
         }
         return {};
      }
      /*virtual*/ QModelIndex PackageProcedureTreeModel::parent(const QModelIndex& index) const /*override*/ {
         if (!index.isValid())
            return {};
         if (index.internalPointer() == this)
            return {};
         auto* parent = (node_type*)index.internalPointer();
         if (!parent) {
            return _qmi_for_model();
         }
         auto* grandparent = parent->parent_node;
         if (!grandparent) {
            auto row = index.row();
            if (row == 0)
               return _qmi_for_root();
            return _qmi_for_orphan(row - 1);
         }
         auto row = grandparent->index_of(*parent);
         return _qmi_for_child(*grandparent, row);
      }
      /*virtual*/ QModelIndex PackageProcedureTreeModel::sibling(int row, int column, const QModelIndex& index) const /*override*/ {
         if (row < 0 || column < 0)
            return {};
         if (!index.isValid())
            return {};
         auto* node = _node_for_qmi(index);
         if (!node)
            return {};
         auto* parent = node->parent_node;
         if (!parent) { // top-level node?
            return this->index(row, column, {});
         }
         return _qmi_for_child(*parent, row, column);
      }
      /*virtual*/ int PackageProcedureTreeModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (parent.model() != this)
            return 0;
         if (_is_model_qmi(parent))
            return this->_orphans.size() + (this->_root ? 1 : 0);
         auto* node = _node_for_qmi(parent);
         if (!node)
            return 0;
         if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node->data)) {
            return casted->children.size();
         }
         return 0;
      }
      /*virtual*/ int PackageProcedureTreeModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant PackageProcedureTreeModel::data(const QModelIndex& index, int role) const /*override*/ {
         auto* node = _node_for_qmi(index);
         if (!node)
            return {};

         auto* branch_data    = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node->data);
         auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);

         switch (role) {
            case BranchTypeRole:
               if (!branch_data)
                  return {};
               return (int)branch_data->type;
            case BranchFlagsRole:
               if (!branch_data)
                  return {};
               return (int)branch_data->flags;
            case ProcedureTypeRole:
               if (!procedure_data)
                  return {};
               return (int)procedure_data->type;
            case ProcedureFlagsRole:
               if (!procedure_data)
                  return {};
               return (int)procedure_data->flags;
            case ProcedureOverrideFlagsRole:
               if (!procedure_data)
                  return {};
               if (!procedure_data->flag_overrides.has_value())
                  return {};
               return QVariant::fromValue(procedure_data->flag_overrides.value());
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               if (branch_data) {
                  return editor::localize::package_procedure_tree_branch_type(branch_data->type);
               } else if (procedure_data) {
                  return tr("Procedure: %1").arg(editor::localize::package_procedure_type(procedure_data->type));
               }
               break;
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags PackageProcedureTreeModel::flags(const QModelIndex& index) const /*override*/ {
         auto  flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
         auto* node  = _node_for_qmi(index);
         if (!node) {
            return flags;
         }
         if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(node->data)) {
            flags |= Qt::ItemNeverHasChildren;
         }
         return flags;
      }
      #pragma region Write-access
         /*virtual*/ bool PackageProcedureTreeModel::setData(const QModelIndex& index, const QVariant& value, int role) /*override*/ {
            auto* node = _node_for_qmi(index);
            if (!node)
               return false;
            auto* branch_data    = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node->data);
            auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
            switch (role) {
               case BranchTypeRole:
                  if (branch_data) {
                     branch_data->type = (branch_type)value.toInt();
                     emit dataChanged(index, index, { BranchTypeRole });
                     return true;
                  }
                  break;
               case BranchFlagsRole:
                  if (branch_data) {
                     branch_data->flags = (std::underlying_type_t<branch_flag::type>)value.toInt();
                     emit dataChanged(index, index, { BranchFlagsRole });
                     return true;
                  }
                  break;
               case ProcedureTypeRole:
                  if (procedure_data) {
                     procedure_data->type = (procedure_type)value.toInt();
                     emit dataChanged(index, index, { ProcedureTypeRole });
                     return true;
                  }
                  break;
               case ProcedureFlagsRole:
                  if (procedure_data) {
                     procedure_data->flags = (std::underlying_type_t<procedure_flag::type>)value.toInt();
                     emit dataChanged(index, index, { ProcedureFlagsRole });
                     return true;
                  }
                  break;
               case ProcedureOverrideFlagsRole:
                  if (procedure_data) {
                     if (value.canConvert<dovah::loaded_forms::structs::custom_packages::package_flag_overrides>()) {
                        procedure_data->flag_overrides = value.value<dovah::loaded_forms::structs::custom_packages::package_flag_overrides>();
                        emit dataChanged(index, index, { ProcedureOverrideFlagsRole });
                        return true;
                     }
                  }
                  break;
            }
            return false;
         }
      #pragma endregion
   #pragma endregion
   /*virtual*/ QVariant PackageProcedureTreeModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      if (orientation != Qt::Orientation::Horizontal)
         return {};
      if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
         return {};
      if (section == 0)
         return tr("Procedure tree");
      return {};
   }
#pragma endregion

void PackageProcedureTreeModel::clear() {
   this->beginResetModel();
   this->_root.reset();
   this->_orphans.clear();
   this->endResetModel();
}

void PackageProcedureTreeModel::import_tree(const backend_type& src) {
   this->beginResetModel();
   this->_root.reset();
   this->_orphans.clear();
   if (src.root) {
      this->_root = std::make_unique<node_type>();
      this->_root->importData(*src.root);
   }
   for (auto& src_orphan_ptr : src.orphans) {
      if (!src_orphan_ptr)
         continue;
      auto& dst_orphan_ptr = this->_orphans.emplace_back(std::make_unique<node_type>());
      dst_orphan_ptr->importData(*src_orphan_ptr);
   }
   this->endResetModel();
}
void PackageProcedureTreeModel::export_tree(backend_type& dst, dovah::loaded_forms::Form& dst_containing_form) const {
   dst.clear(dst_containing_form);
   if (this->_root) {
      dst.root = std::make_unique<node_type::backend_type>();
      this->_root->exportData(*dst.root, dst_containing_form);
   }
   for (auto& src_orphan_ptr : this->_orphans) {
      assert(src_orphan_ptr != nullptr);
      auto& dst_orphan_ptr = dst.orphans.emplace_back(std::make_unique<node_type>());
      src_orphan_ptr->exportData(*dst_orphan_ptr, dst_containing_form);
   }
}

size_t PackageProcedureTreeModel::procedureParameterIDCount(const QModelIndex& qmi) const {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return 0;
   auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
   if (!procedure_data)
      return 0;
   auto  type   = procedure_data->type;
   auto& params = procedure_data->parameter_unique_ids;
   if ((size_t)type >= dovah::packages::all_procedure_type_info.size()) {
      return 0;
   }
   return dovah::packages::all_procedure_type_info[(size_t)type].param_count;
}
uint8_t PackageProcedureTreeModel::getProcedureParameterID(const QModelIndex& qmi, size_t index) const {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return no_unique_id;
   auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
   if (!procedure_data)
      return no_unique_id;
   auto type = procedure_data->type;
   if ((size_t)type >= dovah::packages::all_procedure_type_info.size())
      return no_unique_id;
   if (index >= procedure_data->parameter_unique_ids.size())
      return no_unique_id;
   if (index >= dovah::packages::all_procedure_type_info[(size_t)type].param_count)
      return no_unique_id;
   return procedure_data->parameter_unique_ids[index];
}
std::vector<uint8_t> PackageProcedureTreeModel::getProcedureParameterIDs(const QModelIndex& qmi, size_t index) const {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return {};
   auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
   if (!procedure_data)
      return {};
   auto type = procedure_data->type;
   if ((size_t)type >= dovah::packages::all_procedure_type_info.size()) {
      return {};
   }

   const auto&          src_list = procedure_data->parameter_unique_ids;
   std::vector<uint8_t> dst_list;
   dst_list.resize(dovah::packages::all_procedure_type_info[(size_t)type].param_count, no_unique_id);
   for (size_t i = 0; i < src_list.size(); ++i) {
      dst_list[i] = src_list[i];
   }
   return dst_list;
}
void PackageProcedureTreeModel::setProcedureParameterID(const QModelIndex& qmi, size_t index, uint8_t unique_id) {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return;
   auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
   if (!procedure_data)
      return;

   auto  type   = procedure_data->type;
   auto& params = procedure_data->parameter_unique_ids;
   if ((size_t)type >= dovah::packages::all_procedure_type_info.size()) {
      return;
   }
   size_t count = dovah::packages::all_procedure_type_info[(size_t)type].param_count;
   if (params.size() != count)
      params.resize(count, no_unique_id);
   if (index >= count)
      return;
   params[index] = unique_id;
}

std::vector<ui::types::conditions::condition> PackageProcedureTreeModel::nodeConditions(const QModelIndex& qmi) const {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return {};
   return node->conditions;
}
void PackageProcedureTreeModel::setNodeConditions(const QModelIndex& qmi, const std::vector<ui::types::conditions::condition>& src) {
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return;
   node->conditions = src;
}

void PackageProcedureTreeModel::_sever_uses_of_form(dovah::form_stub& stub) {
   auto traverse = [this, &stub](this auto&& recurse, node_type& node) -> void {
      for (auto& condition : node.conditions) {
         condition.sever_outbound_references_to(&stub);
      }
      if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node.data)) {
         for (auto& child_ptr : casted->children) {
            recurse(*child_ptr);
         }
      }
   };
   if (this->_root)
      traverse(*this->_root);
   for (auto& ptr : this->_orphans)
      if (ptr)
         traverse(*ptr);
}