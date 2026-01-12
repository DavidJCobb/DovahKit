#include "./PackageProcedureTreeModel.h"
#pragma region Drag and drop
   #include <QByteArray>
   #include <QDataStream>
   #include <QMimeData>
#pragma endregion
#include "helpers/bitset.h"
#include "helpers/vectors/move_item_within.h"
#include "dovah/forms/structs/custom_packages/procedure_tree.h"
#include "editor/core.h"
#include "editor/localize/package_procedure_tree_branch_type.h"
#include "editor/localize/package_procedure_type.h"

namespace {
   constexpr const char* const mime_type = "application/dovah-kit.package.procedure-tree-nodes";
}

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
   QModelIndex PackageProcedureTreeModel::_qmi_for_node(const node_type& node) const {
      if (!node.parent_node) {
         if (&node == this->_root.get())
            return _qmi_for_root();
         for (size_t i = 0; i < this->_orphans.size(); ++i)
            if (this->_orphans[i].get() == &node)
               return _qmi_for_orphan(i);
         return {};
      }
      if (auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node.parent_node->data)) {
         auto& siblings = casted->children;
         for (size_t i = 0; i < siblings.size(); ++i)
            if (siblings[i].get() == &node)
               return _qmi_for_child(*node.parent_node, i);
      }
      return {};
   }
   bool PackageProcedureTreeModel::_is_model_qmi(const QModelIndex& qmi) const {
      // QTreeView and friends use, as the root, an invalid QMI with no model set.
      return !qmi.isValid();
   }
   const PackageProcedureTreeModel::node_type* PackageProcedureTreeModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (qmi.model() != this)
         return nullptr;
      auto row = qmi.row();
      if (row < 0)
         return nullptr;
      //
      // The QMI for a given node has, as its internal pointer, the node's parent.
      //
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

bool PackageProcedureTreeModel::_can_move_node_into(const node_type& subject, const node_type& destination) const {
   //
   // Don't allow moving any node into a leaf node:
   //
   if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(destination.data))
      return false;
   //
   // Don't allow moving an ancestor node into itself or its own descendants:
   //
   if (&subject == &destination)
      return false;
   if (subject.contains(destination))
      return false;
   //
   return true;
}
bool PackageProcedureTreeModel::_can_move_nodes_into(const std::vector<node_type*>& subjects, const node_type& destination) const {
   //
   // Don't allow moving any node into a leaf node:
   //
   if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(destination.data))
      return false;
   //
   // Don't allow moving an ancestor node into itself or its own descendants:
   //
   for (auto* node : subjects) {
      if (node == &destination)
         return false;
      if (node->contains(destination))
         return false;
   }
   //
   return true;
}
void PackageProcedureTreeModel::_unchecked_move_node(node_type& subject, node_type& destination, int row) {
   assert(_can_move_node_into(subject, destination));
   auto subject_qmi     = _qmi_for_node(subject);
   auto destination_qmi = _qmi_for_node(destination);
   if (row < 0)
      row = std::get<ui::types::packages::procedure_tree_typed_data::branch>(destination.data).children.size();
   if (!this->beginMoveRows(this->parent(subject_qmi), subject_qmi.row(), subject_qmi.row(), destination_qmi, row))
      return;

   bool moved_root = false;

   std::unique_ptr<node_type> subject_ptr;
   if (auto* prior_parent = subject.parent_node) {
      size_t index = prior_parent->index_of(subject);
      assert(index != (size_t)-1 && "Asymmetrical parent/child relationship between nodes!");
      subject_ptr = prior_parent->take_child(index);
      if (prior_parent == &destination) {
         if (index < row)
            --row;
      }
   } else if (&subject == this->_root.get()) {
      subject_ptr = std::move(this->_root);
      //
      // Shift the first orphan up to become the new root.
      //
      assert(!this->_orphans.empty() && "The only place it's legal to move the root into is an orphan!");
      this->_root = std::move(this->_orphans[0]);
      this->_orphans.erase(this->_orphans.begin());
      moved_root = true;
   } else {
      bool found = false;
      for (size_t i = 0; i < this->_orphans.size(); ++i) {
         auto& o_ptr = this->_orphans[i];
         if (&subject == o_ptr.get()) {
            found = true;
            subject_ptr = std::move(o_ptr);
            this->_orphans.erase(this->_orphans.begin() + i);
         }
      }
      assert(found && "Can't find this node in our model!");
   }
   destination.insert_child(std::move(subject_ptr), row);
   this->endMoveRows();

   if (moved_root && this->_root) {
      //
      // An orphan was promoted to the root node. It should no longer be shown 
      // in red text.
      //
      auto qmi = _qmi_for_root();
      emit dataChanged(qmi, qmi, { Qt::ForegroundRole });
   }
}
   
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
            // The use of `<=` rather than `<` here is intentional, since for QMIs, 
            // row 0 is the root, and rows [1, n] are the orphans; ergo the "end" 
            // row index (i.e. one past the last) is `1 + this->_orphans.size()`.
            //
            if (row <= this->_orphans.size())
               return this->createIndex(row, col, (void*)this);
            return {};
         }
         if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&parent_node->data)) {
            if (row >= casted->children.size())
               return {};
            return _qmi_for_child(*parent_node, row, col);
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
            if (parent == this->_root.get())
               return _qmi_for_root();
            for (size_t i = 0; i < this->_orphans.size(); ++i)
               if (parent == this->_orphans[i].get())
                  return _qmi_for_orphan(i);
            qWarning("Huh? If X has no grandparent, then how is its parent not a top-level node?");
            return {};
         }
         auto row = grandparent->index_of(*parent);
         if (row == (size_t)-1) {
            qWarning("Huh? How is X's parent not a child of X's grandparent?");
            return {};
         }
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
         if (!parent) { // if top-level node
            return this->index(row, column, _qmi_for_model());
         }
         if (const auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&parent->data)) {
            if (row >= casted->children.size())
               return {};
            return _qmi_for_child(*parent, row, column);
         }
         return {};
      }
      /*virtual*/ int PackageProcedureTreeModel::rowCount(const QModelIndex& parent) const /*override*/ {
         if (_is_model_qmi(parent))
            return this->_orphans.size() + (this->_root ? 1 : 0);
         if (parent.model() != this)
            return 0;
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
            case Qt::ForegroundRole:
               //
               // Show orphans (but not their descendants) in red.
               //
               for (const auto& orphan_ptr : this->_orphans) {
                  if (node == orphan_ptr.get()) {
                     return QBrush(QColor(255, 0, 0));
                  }
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
         flags |= Qt::ItemIsDragEnabled;
         if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(node->data)) {
            flags |= Qt::ItemNeverHasChildren;
         } else {
            flags |= Qt::ItemIsDropEnabled;
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
                     this->_default_params_of(*procedure_data);
                     emit dataChanged(index, index, { ProcedureTypeRole });
                     emit procedureParametersDefaulted(index);
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
   #pragma region Drag and drop
      //
      // QAbstractItemModel provides *some* default behaviors for drag-and-drop, but not 
      // in the way we need. It'd be easier for us to just reimplement drags within our 
      // own local model.
      //
      #pragma region Whole-model queries
         /*virtual*/ QStringList PackageProcedureTreeModel::mimeTypes() const /*override*/ {
            return { QString::fromLatin1(mime_type) };
         }
         /*virtual*/ Qt::DropActions PackageProcedureTreeModel::supportedDropActions() const /*override*/ {
            return Qt::DropAction::MoveAction;
         }
      #pragma endregion
      /*virtual*/ QMimeData* PackageProcedureTreeModel::mimeData(const QModelIndexList& indices) const /*override*/ {
         if (indices.count() <= 0)
            return nullptr;
         QByteArray  data;
         QDataStream stream(&data, QIODevice::WriteOnly);
         //
         // Stream begins with our `this` pointer. The pointer is used only for equality 
         // checks on drop (i.e. no moving/copying procedure data across packages) and is 
         // never dereferenced.
         //
         stream << (intptr_t)this;
         //
         for (const QModelIndex& qmi : indices) {
            const auto* node = _node_for_qmi(qmi);
            if (!node)
               continue;
            //
            // QAbstractItemModel's default implementation serializes all itemData for the 
            // node into the stream. We can't do that because some things, like conditions, 
            // are both not serializable *and* require references to objects held elsewhere. 
            // If QAbstractItemModel's implementation is designed to avoid the possibility 
            // of referenced objects being deleted during the drag operation, then trying 
            // to serialize conditions fails that requirement.
            // 
            // We also can't track the lifetime of the drag operation, so we can't, for 
            // example, store a map of unique IDs to QPersistentModelIndexes, because we 
            // wouldn't know when to destroy the QPMIs.
            // 
            // Our solution is to create IDs on demand for nodes that are being dragged, 
            // and serialize those. We never expose direct access to nodes (and thus to the 
            // unique_ptr list of child nodes), so all node removals go through us; we can 
            // invalidate unique IDs properly.
            // 
            // Of course, since we can't track the lifetime of a drag operation, we have to 
            // assume that a request for a node's MIME data is the start of a drag, and we 
            // have to create the ID then. Since the mimeData() getter is const, this is... 
            // a complication.
            //
            stream << this->_drag_and_drop.track(*const_cast<node_type*>(node));
         }
         //
         QMimeData* mime = new QMimeData();
         mime->setData(mime_type, data);
         return mime;
      }
      /*virtual*/ bool PackageProcedureTreeModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) const /*override*/ {
         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((PackageProcedureTreeModel*)this_pointer != this)
               return false;
         }
         if (parent == _qmi_for_model())
            //
            // Don't allow movement of a non-top-level node to the top level.
            //
            return false;

         const auto* parent_node = _node_for_qmi(parent);
         if (!parent_node) // sanity
            return false;
         if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(parent_node->data)) {
            //
            // Don't allow drops onto leaf nodes.
            //
            return false;
         }

         std::vector<node_type*> dragged_nodes;
         while (!stream.atEnd()) {
            decltype(_drag_and_drop)::uid_type id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               dragged_nodes.push_back(node);
         }
         if (!dragged_nodes.size())
            return true;

         //
         // Don't allow dragging an ancestor node into itself or any of its own descendants.
         //
         for (auto* node : dragged_nodes) {
            if (node == parent_node)
               return false;
            if (node->contains(*parent_node))
               return false;
         }

         return true;
      }
      /*virtual*/ bool PackageProcedureTreeModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent) /*override*/ {
         if (!this->canDropMimeData(mime, action, row, column, parent))
            return false;

         QByteArray  data = mime->data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((PackageProcedureTreeModel*)this_pointer != this)
               return false;
         }
         std::vector<node_type*> nodes;
         while (!stream.atEnd()) {
            decltype(_drag_and_drop)::uid_type id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               nodes.push_back(node);
         }
         if (!nodes.size())
            return false;

         auto* destination_parent = _node_for_qmi(parent);
         assert(destination_parent != nullptr);
         for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
            this->_unchecked_move_node(**it, *destination_parent, row);
         return true;
      }
   #pragma endregion
#pragma endregion

void PackageProcedureTreeModel::clear() {
   this->beginResetModel();
   this->_root.reset();
   this->_orphans.clear();
   this->_drag_and_drop.clear();
   this->endResetModel();
}

void PackageProcedureTreeModel::setPackdataModel(PackageDataModel* model) {
   if (this->_packdata_model == model)
      return;
   if (this->_packdata_model) {
      QObject::disconnect(this->_packdata_model.data(), nullptr, this, nullptr);
   }
   this->_packdata_model = model;
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
      auto& dst_orphan_ptr = dst.orphans.emplace_back(std::make_unique<node_type::backend_type>());
      src_orphan_ptr->exportData(*dst_orphan_ptr, dst_containing_form);
   }
}

bool PackageProcedureTreeModel::hasRoot() const {
   return this->_root != nullptr;
}

#pragma region Node contents accessors (besides data())
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
   std::vector<uint8_t> PackageProcedureTreeModel::getProcedureParameterIDs(const QModelIndex& qmi) const {
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
   void PackageProcedureTreeModel::setProcedureParameterIDs(const QModelIndex& qmi, const std::vector<uint8_t>& src) {
      auto* node = _node_for_qmi(qmi);
      if (!node)
         return;
      auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node->data);
      if (!procedure_data)
         return;

      auto type = procedure_data->type;
      procedure_data->parameter_unique_ids = src;
      if ((size_t)type < dovah::packages::all_procedure_type_info.size()) {
         procedure_data->parameter_unique_ids.resize(
            dovah::packages::all_procedure_type_info[(size_t)type].param_count,
            no_unique_id
         );
      }
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
#pragma endregion

std::unordered_map<uint8_t, size_t> PackageProcedureTreeModel::countUsesOfPackdata() const {
   std::unordered_map<uint8_t, size_t> map;

   auto traverse = [&map](this auto&& recurse, node_type& node) -> void {
      const auto* branch_data    = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node.data);
      const auto* procedure_data = std::get_if<ui::types::packages::procedure_tree_typed_data::procedure>(&node.data);
      if (branch_data) {
         for (auto& child_ptr : branch_data->children)
            recurse(*child_ptr);
      } else if (procedure_data) {
         for (auto id : procedure_data->parameter_unique_ids) {
            ++map[id];
         }
      }
   };
   if (this->_root)
      traverse(*this->_root);
   for (auto& ptr : this->_orphans)
      if (ptr)
         traverse(*ptr);

   return map;
}

QModelIndex PackageProcedureTreeModel::_append_node(const QModelIndex& parent, std::unique_ptr<node_type>&& ptr) {
   if (parent.model() != this)
      return {};
   if (_is_model_qmi(parent)) {
      if (this->_root)
         return {};
      this->beginInsertRows(parent, 0, 0);
      this->_root = std::move(ptr);
      this->endInsertRows();
      return _qmi_for_root();
   }
   auto* parent_node = _node_for_qmi(parent);
   if (!parent_node)
      return {};
   if (auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&parent_node->data)) {
      size_t row = casted->children.size();
      this->beginInsertRows(parent, row, row);
      ptr->parent_node = parent_node;
      casted->children.emplace_back(std::move(ptr));
      this->endInsertRows();
      return this->index(row, 0, parent);
   }
   return {};
}
QModelIndex PackageProcedureTreeModel::appendBranch(const QModelIndex& parent) {
   auto  node_ptr  = std::make_unique<node_type>();
   auto& node_data = node_ptr->data.emplace<ui::types::packages::procedure_tree_typed_data::branch>();
   return _append_node(parent, std::move(node_ptr));
}
QModelIndex PackageProcedureTreeModel::appendProcedure(const QModelIndex& parent) {
   auto  node_ptr  = std::make_unique<node_type>();
   auto& node_data = node_ptr->data.emplace<ui::types::packages::procedure_tree_typed_data::procedure>();
   node_data.type = procedure_type::travel;
   this->_default_params_of(node_data);
   return _append_node(parent, std::move(node_ptr));
}
void PackageProcedureTreeModel::removeItem(const QModelIndex& qmi) {
   if (qmi.model() != this)
      return;
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return;
   this->_on_node_destroyed(*node);
   if (!node->parent_node) {
      if (node == this->_root.get()) {
         this->beginRemoveRows(_qmi_for_model(), 0, 0);
         this->_root.reset();
         if (!this->_orphans.empty()) {
            this->_root = std::move(this->_orphans[0]);
            this->_orphans.erase(this->_orphans.begin());
         }
         this->endRemoveRows();
         if (this->_root) {
            //
            // An orphan was promoted to the root node. It should no longer be shown 
            // in red text.
            //
            auto qmi = _qmi_for_root();
            emit dataChanged(qmi, qmi, { Qt::ForegroundRole });
         }
         return;
      }
      for (size_t i = 0; i < this->_orphans.size(); ++i) {
         auto& ptr = this->_orphans[i];
         if (ptr.get() != node)
            continue;
         this->beginRemoveRows(_qmi_for_model(), i + 1, i + 1);
         this->_orphans.erase(this->_orphans.begin() + i);
         this->endRemoveRows();
         break;
      }
      return;
   }
   if (auto* casted = std::get_if<ui::types::packages::procedure_tree_typed_data::branch>(&node->parent_node->data)) {
      auto& siblings = casted->children;
      for (size_t i = 0; i < siblings.size(); ++i) {
         if (siblings[i].get() != node)
            continue;
         this->beginRemoveRows(_qmi_for_node(*node->parent_node), i, i);
         siblings.erase(siblings.begin() + i);
         this->endRemoveRows();
         break;
      }
   }
}

QModelIndex PackageProcedureTreeModel::wrapInBranch(const QModelIndex& subject_qmi_prior) {
   auto* subject_node = _node_for_qmi(subject_qmi_prior);
   if (!subject_node)
      return {};

   QModelIndex parent_qmi;
   node_type*  parent_node = nullptr;
   std::unique_ptr<node_type>* subject_ptr_p = nullptr;
   bool subject_was_orphan = false;

   if (subject_node == this->_root.get()) {
      subject_ptr_p = &this->_root;
      parent_qmi    = _qmi_for_model();
   } else if (parent_node = subject_node->parent_node) {
      size_t i = parent_node->index_of(*subject_node);
      if (i == (size_t)-1)
         return {};
      parent_qmi    = _qmi_for_node(*parent_node);
      subject_ptr_p = &std::get<ui::types::packages::procedure_tree_typed_data::branch>(parent_node->data).children[i];
   } else {
      parent_qmi = _qmi_for_model();
      for (auto& orphan_ptr : this->_orphans) {
         if (orphan_ptr.get() == subject_node) {
            subject_ptr_p      = &orphan_ptr;
            subject_was_orphan = true;
            break;
         }
      }
   }
   assert(subject_ptr_p != nullptr);

   emit layoutAboutToBeChanged({ parent_qmi });
   auto subject_ptr = std::move(*subject_ptr_p);
   *subject_ptr_p = std::make_unique<node_type>();
   node_type& wrapper_node = *(subject_ptr_p->get());
   wrapper_node.parent_node = parent_node;
   wrapper_node.data.emplace<ui::types::packages::procedure_tree_typed_data::branch>();
   subject_ptr->parent_node = nullptr;
   wrapper_node.append_child(std::move(subject_ptr));

   QModelIndex subject_qmi_after = this->_qmi_for_node(*subject_node);
   this->changePersistentIndex(subject_qmi_prior, subject_qmi_after);
   emit layoutChanged({ _qmi_for_model() });
   if (subject_was_orphan) {
      //
      // Subject is no longer an orphan node (the wrapper now is), so ensure its text 
      // color updates.
      //
      emit dataChanged(subject_qmi_after, subject_qmi_after, { Qt::ForegroundRole });
   }

   return _qmi_for_node(wrapper_node);
}

void PackageProcedureTreeModel::_default_params_of(ui::types::packages::procedure_tree_typed_data::procedure& dst) {
   if ((size_t)dst.type >= dovah::packages::all_procedure_type_info.size())
      return;
   const auto& info = dovah::packages::all_procedure_type_info[(size_t)dst.type];

   auto& dst_list = dst.parameter_unique_ids;
   dst_list.clear();
   dst_list.resize(info.param_count, no_unique_id);

   if (!this->_packdata_model)
      return;

   size_t packdata_count = this->_packdata_model->rowCount();
   for (size_t i = 0; i < info.param_count; ++i) {
      auto& param = info.params[i];

      uint8_t unique_id = no_unique_id;
      for (size_t j = 0; j < packdata_count; ++j) {
         auto item_qmi  = this->_packdata_model->index(j, 0, {});
         auto item_id   = this->_packdata_model->data(item_qmi, PackageDataModel::UniqueIDRole).toInt();
         auto type_data = this->_packdata_model->data(item_qmi, PackageDataModel::TypeRole);
         if (!type_data.isValid())
            continue;
         auto type = (dovah::packages::package_data_type)type_data.toInt();
         switch (param.type) {
            case dovah::packages::procedure_type_info::param_type::boolean:
               if (type == dovah::packages::package_data_type::boolean)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::float32:
               if (type == dovah::packages::package_data_type::float32)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::integer:
               if (type == dovah::packages::package_data_type::integer)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::location:
               if (type == dovah::packages::package_data_type::location)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::object_list:
               if (type == dovah::packages::package_data_type::object_list)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::target_selector:
               if (type == dovah::packages::package_data_type::target_selector)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::target:
               if (type == dovah::packages::package_data_type::single_ref)
                  unique_id = item_id;
               if (type == dovah::packages::package_data_type::target_selector)
                  unique_id = item_id;
               break;
            case dovah::packages::procedure_type_info::param_type::topic:
               if (type == dovah::packages::package_data_type::topic)
                  unique_id = item_id;
               break;
         }
         if (unique_id)
            break;
      }
      if (unique_id == no_unique_id) {
         auto qmi = this->_packdata_model->appendRow();
         if (qmi.isValid()) {
            auto decl = this->_packdata_model->rowDeclaration(qmi.row());
            decl.name      = QString(QByteArray(param.name.data(), param.name.size()));
            decl.is_public = true;
            unique_id = decl.unique_id;

            ui::types::packages::package_data_value value;
            switch (param.type) {
               case dovah::packages::procedure_type_info::param_type::boolean:
               default:
                  value.emplace<dovah::packages::package_data_type::boolean>();
                  break;
               case dovah::packages::procedure_type_info::param_type::float32:
                  value.emplace<dovah::packages::package_data_type::float32>();
                  break;
               case dovah::packages::procedure_type_info::param_type::integer:
                  value.emplace<dovah::packages::package_data_type::integer>();
                  break;
               case dovah::packages::procedure_type_info::param_type::location:
                  value.emplace<dovah::packages::package_data_type::location>();
                  break;
               case dovah::packages::procedure_type_info::param_type::object_list:
                  value.emplace<dovah::packages::package_data_type::object_list>();
                  break;
               case dovah::packages::procedure_type_info::param_type::target_selector:
                  value.emplace<dovah::packages::package_data_type::target_selector>();
                  break;
               case dovah::packages::procedure_type_info::param_type::target:
                  value.emplace<dovah::packages::package_data_type::single_ref>();
                  break;
               case dovah::packages::procedure_type_info::param_type::topic:
                  value.emplace<dovah::packages::package_data_type::topic>();
                  break;
            }
            this->_packdata_model->setRowValue(qmi.row(), value);
         }
      }
      dst_list[i] = unique_id;
   }
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

void PackageProcedureTreeModel::_on_node_destroyed(node_type& node) {
   [this](this auto&& recurse, node_type& node) -> void {
      this->_drag_and_drop.untrack(node);
      if (!std::holds_alternative<ui::types::packages::procedure_tree_typed_data::branch>(node.data))
         return;
      auto& branch = std::get<ui::types::packages::procedure_tree_typed_data::branch>(node.data);
      for (auto& child_ptr : branch.children) {
         recurse(*child_ptr);
      }
   }(node);
}