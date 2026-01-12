#include "./RegionObjectsModel.h"
#include <memory>
#include <vector>
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_stub_drag_drop.h"
#include "ui/model_utils/drag_drop_nodes_by_id.h"
#include "ui/types/regions/region.h"

namespace {
   constexpr const char* const mime_type = "application/dovah-kit.region-objects-model.node";
}

RegionObjectsModel::RegionObjectsModel(QObject* parent) : QAbstractItemModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) { this->_on_form_deleted(*stub); });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) { this->_on_form_modified(*stub); });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() { this->clear(); });
}
RegionObjectsModel::~RegionObjectsModel() {
   this->_cache.clear();
   this->_drag_and_drop.clear();
   this->_tree.clear();
}

#pragma region Node utils
   const RegionObjectsModel::node_type* RegionObjectsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (qmi.model() != this || !qmi.isValid())
         return nullptr;
      auto* parent = (node_type*)qmi.internalPointer();
      if (parent) {
         if (qmi.row() >= parent->children.size())
            return nullptr;
         return parent->children[qmi.row()].get();
      }
      if (qmi.row() >= this->_tree.objects.size())
         return nullptr;
      return this->_tree.objects[qmi.row()].get();
   }
   RegionObjectsModel::node_type* RegionObjectsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<node_type*>(std::as_const(*this)._node_for_qmi(qmi));
   }
   QModelIndex RegionObjectsModel::_qmi_for_node(const node_type& node) const {
      size_t i = (size_t)-1;

      const std::vector<std::unique_ptr<node_type>>* siblings = nullptr;
      if (node.parent)
         siblings = &node.parent->children;
      else
         siblings = &this->_tree.objects;

      for (size_t j = 0; j < siblings->size(); ++j) {
         if ((*siblings)[j].get() == &node) {
            i = j;
            break;
         }
      }
      assert(i != (size_t)-1);
      return this->createIndex(i, 0, node.parent);
   }
#pragma endregion

/*static*/ bool RegionObjectsModel::allows_form_type(dovah::form_type ft) {
   return dovah::form_type_is_base_form(ft);
}

#pragma region QAbstractItemModel /*override*/s
   #pragma region Hierarchy
      /*virtual*/ QModelIndex RegionObjectsModel::index(int row, int column, const QModelIndex& parent) const /*override*/ {
         if (row < 0 || column < 0 || column >= ColumnCount)
            return {};
         auto* node = _node_for_qmi(parent);
         if (node) {
            if (row >= node->children.size())
               return {};
            return _qmi_for_node(*node->children[row]);
         }
         if (row >= node->children.size())
            return {};
         return _qmi_for_node(*node->children[row]);
      }
      /*virtual*/ QModelIndex RegionObjectsModel::parent(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return {};
         auto* node = _node_for_qmi(qmi);
         if (!node || !node->parent)
            return {};
         return _qmi_for_node(*node->parent);
      }
      /*virtual*/ QModelIndex RegionObjectsModel::sibling(int row, int column, const QModelIndex& qmi) const /*override*/ {
         if (row < 0 || column < 0 || column >= ColumnCount)
            return {};
         auto* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         const std::vector<std::unique_ptr<node_type>>* siblings = nullptr;
         if (node->parent) {
            siblings = &node->parent->children;
         } else {
            siblings = &this->_tree.objects;
         }
         if (row >= siblings->size())
            return {};
         return _qmi_for_node(*(*siblings)[row]);
      }
      /*virtual*/ int RegionObjectsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         auto* node = _node_for_qmi(parent);
         if (!node)
            return this->_tree.objects.size();
         return node->children.size();
      }
      /*virtual*/ int RegionObjectsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RegionObjectsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         const node_type* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               {
                  auto it = this->_cache.find(const_cast<node_type*>(node)); // can't use const pointers for lookups when non-const pointers are the key type -_-
                  if (it != this->_cache.end()) {
                     auto& cache = it->second;
                     return cache.editor_id;
                  }
               }
               break;
            case ObjectDataRole:
               return QVariant::fromValue(node->data);
         }
         return {};
      }
      /*virtual*/ Qt::ItemFlags RegionObjectsModel::flags(const QModelIndex& qmi) const /*override*/ {
         if (!qmi.isValid())
            return Qt::ItemFlag::ItemIsDropEnabled;
         auto flags = Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsDropEnabled;
         return flags;
      }
      /*virtual*/ bool RegionObjectsModel::setData(const QModelIndex& qmi, const QVariant& value, int role) /*override*/ {
         if (qmi.column() >= ColumnCount)
            return false;
         auto* node = _node_for_qmi(qmi);
         if (!node)
            return false;
         switch (role) {
            case ObjectDataRole:
               {
                  if (!value.canConvert<object_data>())
                     return false;
                  auto data = value.value<object_data>();
                  if (!data.base_form)
                     return false;

                  auto* prior_stub = node->data.base_form;
                  node->data = data;
                  if (node->data.base_form != prior_stub) {
                     this->_recache_node(*node, false);
                  }
               }
               break;
         }
         return false;
      }
   #pragma endregion
   /*virtual*/ QVariant RegionObjectsModel::headerData(int section, Qt::Orientation orientation, int role) const /*override*/ {
      return {};
   }
   #pragma region Drag and drop
      #pragma region Whole-model queries
         /*virtual*/ QStringList RegionObjectsModel::mimeTypes() const /*override*/ {
            return QStringList({ mime_type });
         }
         /*virtual*/ Qt::DropActions RegionObjectsModel::supportedDragActions() const /*override*/ {
            return Qt::MoveAction;
         }
         /*virtual*/ Qt::DropActions RegionObjectsModel::supportedDropActions() const /*override*/ {
            //
            // Allow copying when what's being dragged is a list of form stubs from the Object 
            // Window. Allow moving when what's being dragged is nodes from inside of this very 
            // model.
            //
            return Qt::CopyAction | Qt::MoveAction;
         }
      #pragma endregion
      /*virtual*/ QMimeData* RegionObjectsModel::mimeData(const QModelIndexList& indices) const /*override*/ {
         return _get_node_drag_data(indices);
      }
      /*virtual*/ bool RegionObjectsModel::canDropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent_qmi) const /*override*/ {
         if (column >= ColumnCount || row > this->rowCount(parent_qmi))
            return false;
         if (!mime)
            return false;

         if (_is_dragged_form_stub_list(*mime)) {
            return _can_drop_form_stub_list(*mime, action);
         }
         if (_is_dragged_nodes(*mime)) {
            return _can_drop_nodes(*mime, action, row, parent_qmi);
         }

         return false;
      }
      /*virtual*/ bool RegionObjectsModel::dropMimeData(const QMimeData* mime, Qt::DropAction action, int row, int column, const QModelIndex& parent_qmi) /*override*/ {
         if (!this->canDropMimeData(mime, action, row, column, parent_qmi))
            return false;
         if (action == Qt::IgnoreAction)
            return true;
         auto* parent_node = _node_for_qmi(parent_qmi);
         if (row == -1) {
            row = parent_node ? parent_node->children.size() : this->_tree.objects.size();
         }

         if (_is_dragged_form_stub_list(*mime))
            return _drop_form_stub_list(*mime, action, parent_qmi, parent_node, row);

         if (_is_dragged_nodes(*mime))
            return _drop_nodes(*mime, action, parent_qmi, parent_node, row);

         return false;
      }
   #pragma endregion
#pragma endregion

void RegionObjectsModel::importData(const frontend_form_data& region) {
   this->beginResetModel();
   this->_cache.clear();
   this->_drag_and_drop.clear();
   this->_tree.clear();

   auto& src_coll_opt = region.generable_content.objects;
   if (src_coll_opt.has_value()) {
      auto& src_coll = src_coll_opt.value();
      this->_tree = src_coll;
   }

   auto crawl = [this](this auto&& recurse, node_type& node) -> void {
      this->_recache_node(node, true);
      for (auto& child_ptr : node.children) {
         recurse(*child_ptr);
      }
   };
   for (auto& node_ptr : this->_tree.objects) {
      crawl(*node_ptr);
   }

   this->endResetModel();
}
void RegionObjectsModel::exportData(frontend_form_data& region) const {
   auto& dst_coll_opt = region.generable_content.objects;
   dst_coll_opt.emplace();

   auto& dst_coll = dst_coll_opt.value();
   dst_coll.tree_type::operator=(this->_tree);
}
void RegionObjectsModel::clear() {
   this->beginResetModel();
   this->_cache.clear();
   this->_drag_and_drop.clear();
   this->_tree.clear();
   this->endResetModel();
}

QModelIndex RegionObjectsModel::insertObject(const QModelIndex& parent_qmi, int row, dovah::form_stub& base_form) {
   object_data data;
   data.base_form = &base_form;
   return this->insertObject(parent_qmi, row, data);
}
QModelIndex RegionObjectsModel::insertObject(const QModelIndex& parent_qmi, int row, const object_data& data) {
   if (row < 0)
      return {};
   if (!data.base_form || !allows_form_type(data.base_form->form_type))
      return {};
   auto* parent_node = _node_for_qmi(parent_qmi);
   auto* siblings    = &this->_tree.objects;
   if (parent_node) {
      siblings = &parent_node->children;
   } else {
      if (parent_qmi.isValid())
         return {}; // bad QMI, not root QMI
   }
   if (row > siblings->size())
      return {};

   this->beginInsertRows(parent_qmi, row, row);
   auto node_ptr = std::make_unique<node_type>();
   node_ptr->data   = data;
   node_ptr->parent = parent_node;
   siblings->insert(siblings->begin() + row, std::move(node_ptr));
   this->endInsertRows();

   return this->createIndex(row, 0, parent_node);
}
void RegionObjectsModel::removeObject(const QModelIndex& qmi) {
   if (!qmi.isValid() || qmi.model() != this)
      return;
   auto* node = _node_for_qmi(qmi);
   if (!node)
      return;

   QModelIndex parent_qmi;
   auto*       siblings = &this->_tree.objects;
   int         row      = -1;
   if (node->parent) {
      parent_qmi = _qmi_for_node(*node->parent);
      siblings   = &node->parent->children;
      row        = node->parent->index_of(*node);
   } else {
      row = this->_tree.index_of(*node);
   }
   assert(row != node_type::index_of_none);

   this->beginRemoveRows(parent_qmi, row, row);
   siblings->erase(siblings->begin() + row);
   this->endRemoveRows();
}
void RegionObjectsModel::removeObjects(const QModelIndex& parent_qmi, size_t row, size_t count) {
   if (count == 0)
      return;
   auto* siblings = &this->_tree.objects;
   if (parent_qmi.isValid()) {
      auto* parent_node = _node_for_qmi(parent_qmi);
      if (!parent_node)
         return;
      siblings = &parent_node->children;
   }
   if (row + count > siblings->size())
      return;
   
   this->beginRemoveRows(parent_qmi, row, row + count - 1);
   siblings->erase(siblings->begin() + row, siblings->begin() + row + count);
   this->endRemoveRows();
}

void RegionObjectsModel::_on_node_destroyed(node_type& node) {
   this->_drag_and_drop.untrack(node);
   [this](this auto&& recurse, node_type& node) -> void {
      this->_cache.erase(&node);
      for (auto& child_ptr : node.children)
         recurse(*child_ptr);
   }(node);
}

void RegionObjectsModel::_on_form_deleted(dovah::form_stub& stub) {
   if (!allows_form_type(stub.form_type))
      return;

   auto crawl = [this, &stub](this auto&& recurse, node_type& parent) -> void {
      QModelIndex parent_qmi;
      auto&  list = parent.children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* child = list[i].get();
         if (child->data.base_form != &stub) {
            recurse(*child);
            continue;
         }
         if (!parent_qmi.isValid())
            parent_qmi = _qmi_for_node(parent);
         this->beginRemoveRows(parent_qmi, i, i);
         this->_on_node_destroyed(*child);
         list.erase(list.begin() + i);
         --i;
         --size;
         this->endRemoveRows();
      }
   };

   auto&  list = this->_tree.objects;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* child = list[i].get();
      if (child->data.base_form == &stub) {
         this->beginRemoveRows({}, i, i);
         this->_on_node_destroyed(*child);
         list.erase(list.begin() + i);
         --i;
         --size;
         this->endRemoveRows();
      } else {
         crawl(*child);
      }
   }
}
void RegionObjectsModel::_on_form_modified(dovah::form_stub& stub) {
   QString editor_id;

   auto crawl = [this, &editor_id, &stub](this auto&& recurse, node_type& object) -> void {
      if (object.data.base_form == &stub) {
         _recache_node(object, false);
      }
      for (auto& child_ptr : object.children) {
         recurse(*child_ptr);
      }
   };

   for (auto& node_ptr : this->_tree.objects)
      crawl(*node_ptr);
}

void RegionObjectsModel::_recache_node(node_type& node, bool silent) {
   auto& cache = this->_cache[&node];
   cache.editor_id = tr("NONE");
   if (auto* stub = node.data.base_form) {
      cache.editor_id = QString::fromStdString(stub->editorID);
   }
   if (!silent) {
      auto qmi = _qmi_for_node(node);
      emit dataChanged(qmi, qmi);
   }
}

#pragma region Drag and drop implementation
   #pragma region Forms from the Object Window
      /*static*/ bool RegionObjectsModel::_is_dragged_form_stub_list(const QMimeData& mime) {
         return mime.hasFormat(editor_helpers::form_stub_array_mime_type);
      }
      bool RegionObjectsModel::_can_drop_form_stub_list(const QMimeData& mime, Qt::DropAction action) const {
         if (!_is_dragged_form_stub_list(mime))
            return false;
         if (action != Qt::DropAction::CopyAction)
            return false;
         auto list = editor_helpers::form_stubs_from_mime_data(mime);
         for (auto* stub : list) {
            if (stub && allows_form_type(stub->form_type)) {
               return true;
            }
         }
         return false;
      }
      bool RegionObjectsModel::_drop_form_stub_list(const QMimeData& mime, Qt::DropAction action, const QModelIndex& parent_qmi, node_type* parent_node, int row) {
         auto dropped_stubs = editor_helpers::form_stubs_from_mime_data(mime);
         std::erase_if(
            dropped_stubs,
            [this](dovah::form_stub* stub) -> bool {
               return stub && allows_form_type(stub->form_type);
            }
         );
         if (dropped_stubs.empty())
            return false;

         this->beginInsertRows(parent_qmi, row, row + dropped_stubs.size() - 1);
         for (size_t i = 0; i < dropped_stubs.size(); ++i) {
            auto  item_ptr = std::make_unique<node_type>();
            auto& item     = *item_ptr;
            if (parent_node) {
               parent_node->children.insert(parent_node->children.begin() + row + i, std::move(item_ptr));
            } else {
               this->_tree.objects.insert(this->_tree.objects.begin() + row + i, std::move(item_ptr));
            }
            item.data.base_form = dropped_stubs[i];
            this->_recache_node(item, true);
         }
         this->endInsertRows();
         return true;
      }
   #pragma endregion
   #pragma region Drag-moving nodes within our tree
      QMimeData* RegionObjectsModel::_get_node_drag_data(const QModelIndexList& indices) const {
         if (indices.count() <= 0)
            return nullptr;

         QByteArray data = ui::model_utils::drag_drop_nodes_by_id::build_data(
            indices,
            *this,
            this->_drag_and_drop,
            [this](const QModelIndex& qmi) {
               return _node_for_qmi(qmi);
            }
         );
         QMimeData* mime = new QMimeData();
         mime->setData(mime_type, data);
         return mime;
      }
      /*static*/ bool RegionObjectsModel::_is_dragged_nodes(const QMimeData& mime) {
         return mime.hasFormat(mime_type);
      }
      bool RegionObjectsModel::_can_drop_nodes(const QMimeData& mime, Qt::DropAction action, int row, const QModelIndex& parent_qmi) const {
         if (action != Qt::DropAction::MoveAction)
            return false;

         auto extracted = ui::model_utils::drag_drop_nodes_by_id::extract_nodes_from_data(
            mime.data(mime_type),
            *this,
            this->_drag_and_drop
         );
         if (!extracted.has_value())
            return false;
         auto& dragged_nodes = extracted.value();

         const auto* parent_node = _node_for_qmi(parent_qmi);
         if (!parent_node)
            return true;

         //
         // Don't allow dragging any node into itself or its own descendants.
         //
         for (auto* node : dragged_nodes) {
            if (node == parent_node)
               return false;
            if (node->contains(*parent_node))
               return false;
         }

         return true;
      }
      bool RegionObjectsModel::_drop_nodes(const QMimeData& mime, Qt::DropAction action, const QModelIndex& dst_parent_qmi, node_type* dst_parent_node, int row) {
         auto extracted = ui::model_utils::drag_drop_nodes_by_id::extract_nodes_from_data(
            mime.data(mime_type),
            *this,
            this->_drag_and_drop
         );
         if (!extracted.has_value())
            return false;
         auto& nodes = extracted.value();
         if (!nodes.size())
            return false;

         auto* dst_siblings = &this->_tree.objects;
         if (dst_parent_node) {
            dst_siblings = &dst_parent_node->children;
         }
         for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
            node_type& node = **it;

            auto*       src_siblings = &this->_tree.objects;
            QModelIndex src_parent_qmi;
            if (node.parent) {
               src_siblings   = &node.parent->children;
               src_parent_qmi = _qmi_for_node(*node.parent);
            }
            int src_i = -1;
            for (size_t i = 0; i < src_siblings->size(); ++i) {
               if ((*src_siblings)[i].get() == &node) {
                  src_i = i;
                  break;
               }
            }
            assert(src_i >= 0);

            this->beginMoveRows(src_parent_qmi, src_i, src_i, dst_parent_qmi, row);
            dst_siblings->insert(dst_siblings->begin() + row, std::move((*src_siblings)[src_i]));
            src_siblings->erase(src_siblings->begin() + src_i);
            this->endMoveRows();
         }
         return true;
      }
   #pragma endregion
#pragma endregion