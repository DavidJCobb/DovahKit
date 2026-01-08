#include "./RegionObjectsModel.h"
#include <memory>
#include "dovah/forms/Region.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"
#include "editor/helpers/form_stub_drag_drop.h"

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
   this->_drag_and_drop.clear();
   for (auto*& ptr : this->_data) {
      if (ptr) {
         delete ptr;
         ptr = nullptr;
      }
   }
   this->_data.clear();
}

#pragma region RegionObjectsModel::Object
   RegionObjectsModel::Object::~Object() {
      for (auto*& ptr : this->children) {
         if (ptr) {
            delete ptr;
            ptr = nullptr;
         }
      }
      this->children.clear();
   }
#pragma endregion

#pragma region Node utils
   const RegionObjectsModel::Object* RegionObjectsModel::_node_for_qmi(const QModelIndex& qmi) const {
      if (qmi.model() != this || !qmi.isValid())
         return nullptr;
      auto* parent = (Object*)qmi.internalPointer();
      if (parent) {
         if (qmi.row() >= parent->children.size())
            return nullptr;
         return parent->children[qmi.row()];
      }
      if (qmi.row() >= this->_data.size())
         return nullptr;
      return this->_data[qmi.row()];
   }
   RegionObjectsModel::Object* RegionObjectsModel::_node_for_qmi(const QModelIndex& qmi) {
      return const_cast<Object*>(std::as_const(*this)._node_for_qmi(qmi));
   }
   QModelIndex RegionObjectsModel::_qmi_for_node(const Object& node) const {
      size_t i = (size_t)-1;

      const std::vector<Object*>* siblings = nullptr;
      if (node.parent)
         siblings = &node.parent->children;
      else
         siblings = &this->_data;

      for (size_t j = 0; j < siblings->size(); ++j) {
         if ((*siblings)[j] == &node) {
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
         const std::vector<Object*>* siblings = nullptr;
         if (node->parent) {
            siblings = &node->parent->children;
         } else {
            siblings = &this->_data;
         }
         if (row >= siblings->size())
            return {};
         return _qmi_for_node(*(*siblings)[row]);
      }
      /*virtual*/ int RegionObjectsModel::rowCount(const QModelIndex& parent) const /*override*/ {
         auto* node = _node_for_qmi(parent);
         if (!node)
            return this->_data.size();
         return node->children.size();
      }
      /*virtual*/ int RegionObjectsModel::columnCount(const QModelIndex& parent) const /*override*/ {
         return ColumnCount;
      }
      #pragma region Editing
         /*virtual*/ bool RegionObjectsModel::removeRows(int row, int count, const QModelIndex& parent_qmi) /*override*/ {
            if (row < 0 || count <= 0)
               return false;
            auto* siblings = &this->_data;
            if (parent_qmi.isValid()) {
               auto* parent_node = _node_for_qmi(parent_qmi);
               if (!parent_node)
                  return false;
               siblings = &parent_node->children;
            }
            if (row + count > siblings->size())
               return false;

            this->beginRemoveRows(parent_qmi, row, row + count - 1);
            for (size_t i = 0; i < count; ++i) {
               auto& node_ptr = (*siblings)[row + i];
               delete node_ptr;
               node_ptr = nullptr;
            }
            siblings->erase(siblings->begin() + row, siblings->begin() + row + count);
            this->endRemoveRows();
            return true;
         }
      #pragma endregion
   #pragma endregion
   #pragma region Node data
      /*virtual*/ QVariant RegionObjectsModel::data(const QModelIndex& qmi, int role) const /*override*/ {
         auto* node = _node_for_qmi(qmi);
         if (!node)
            return {};
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return node->cached.editor_id;
            case ObjectDataRole:
               return QVariant::fromValue(*(ObjectData*)node);
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
                  if (!value.canConvert<ObjectData>())
                     return false;
                  auto data = value.value<ObjectData>();
                  if (!data.form)
                     return false;

                  auto* prior_stub = node->form;
                  node->ObjectData::operator=(std::move(data));
                  if (node->form != prior_stub) {
                     node->cached.editor_id = QString::fromStdString(node->form->editorID);
                     emit dataChanged(qmi, qmi);
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
            return QStringList({
               //editor_helpers::form_stub_array_mime_type,
               mime_type
            });
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
            row = parent_node ? parent_node->children.size() : this->_data.size();
         }

         if (_is_dragged_form_stub_list(*mime))
            return _drop_form_stub_list(*mime, action, parent_qmi, parent_node, row);

         if (_is_dragged_nodes(*mime))
            return _drop_nodes(*mime, action, parent_qmi, parent_node, row);

         return false;
      }
   #pragma endregion
#pragma endregion

void RegionObjectsModel::importData(const backend_collection_type& src_coll) {
   this->beginResetModel();
   for (auto*& ptr : this->_data) {
      if (ptr) {
         delete ptr;
         ptr = nullptr;
      }
   }
   this->_data.clear();

   std::vector<Object*> flat_list;
   flat_list.reserve(src_coll.objects.size());
   for (size_t i = 0; i < src_coll.objects.size(); ++i) {
      auto& src_item     = src_coll.objects[i];
      if (!src_item.form)
         continue;
      if (!allows_form_type(src_item.form.get_form_stub()->form_type))
         continue;
      auto  dst_item_ptr = std::make_unique<Object>();
      auto& dst_item     = *dst_item_ptr;
      dst_item.form   = src_item.form.get_form_stub();
      dst_item.params = src_item.params;
      if (src_item.parent_index < 0 || src_item.parent_index >= flat_list.size()) {
         this->_data.push_back(&dst_item);
      } else {
         auto* parent = flat_list[src_item.parent_index];
         parent->children.push_back(&dst_item);
         dst_item.parent = parent;
      }
      flat_list.push_back(&dst_item);
      dst_item_ptr.release();
   }

   this->endResetModel();
}
void RegionObjectsModel::exportData(backend_collection_type& dst_coll, loaded_form_type& dst_form) const {
   dst_coll.clear(dst_form);

   for (auto* item : this->_data) {
      [&dst_coll, &dst_form](this auto&& recurse, Object& subject, int parent_index) -> void {
         size_t subject_index = dst_coll.objects.size();
         auto&  dst_item      = dst_coll.objects.emplace_back();
         dst_item.form.set(dst_form, subject.form);
         dst_item.parent_index = parent_index;
         dst_item.params       = subject.params;
         for (auto* child : subject.children) {
            recurse(*child, subject_index);
         }
      }(*item, -1);
   }
}
void RegionObjectsModel::clear() {
   this->beginResetModel();
   this->_drag_and_drop.clear();
   for (auto*& ptr : this->_data) {
      if (ptr) {
         delete ptr;
         ptr = nullptr;
      }
   }
   this->_data.clear();
   this->endResetModel();
}

QModelIndex RegionObjectsModel::insertObject(const QModelIndex& parent_qmi, int row, dovah::form_stub& base_form) {
   ObjectData data;
   data.form = &base_form;
   return this->insertObject(parent_qmi, row, data);
}
QModelIndex RegionObjectsModel::insertObject(const QModelIndex& parent_qmi, int row, const ObjectData& data) {
   if (row < 0)
      return {};
   if (!data.form || !allows_form_type(data.form->form_type))
      return {};
   auto* parent_node = _node_for_qmi(parent_qmi);
   auto* siblings    = &this->_data;
   if (parent_node) {
      siblings = &parent_node->children;
   } else {
      if (parent_qmi.isValid())
         return {}; // bad QMI, not root QMI
   }
   if (row > siblings->size())
      return {};

   this->beginInsertRows(parent_qmi, row, row);
   auto node_ptr = std::make_unique<Object>();
   node_ptr->ObjectData::operator=(data);
   node_ptr->parent = parent_node;
   siblings->insert(siblings->begin() + row, &*node_ptr);
   node_ptr.release();
   this->endInsertRows();

   return this->createIndex(row, 0, parent_node);
}

void RegionObjectsModel::_on_form_deleted(dovah::form_stub& stub) {
   if (!allows_form_type(stub.form_type))
      return;

   auto crawl = [this, &stub](this auto&& recurse, Object& parent) -> void {
      QModelIndex parent_qmi;
      auto&  list = parent.children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         auto* child = list[i];
         if (child->form != &stub) {
            recurse(*child);
            continue;
         }
         if (!parent_qmi.isValid())
            parent_qmi = _qmi_for_node(parent);
         this->beginRemoveRows(parent_qmi, i, i);
         list.erase(list.begin() + i);
         this->_drag_and_drop.on_node_destroyed(*child);
         delete child;
         --i;
         --size;
         this->endRemoveRows();
      }
   };

   auto&  list = this->_data;
   size_t size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* child = list[i];
      if (child->form == &stub) {
         this->beginRemoveRows({}, i, i);
         list.erase(list.begin() + i);
         this->_drag_and_drop.on_node_destroyed(*child);
         delete child;
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

   auto crawl = [this, &editor_id, &stub](this auto&& recurse, Object& object) -> void {
      if (object.form == &stub) {
         if (editor_id.isEmpty()) {
            editor_id = QString::fromStdString(stub.editorID);
         }
         object.cached.editor_id = editor_id;
         auto qmi = _qmi_for_node(object);
         emit dataChanged(qmi, qmi);
      }
      for (auto* child : object.children) {
         recurse(*child);
      }
   };

   for (auto* node : this->_data)
      crawl(*node);
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
      bool RegionObjectsModel::_drop_form_stub_list(const QMimeData& mime, Qt::DropAction action, const QModelIndex& parent_qmi, Object* parent_node, int row) {
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
            auto  item_ptr = std::make_unique<Object>();
            auto& item     = *item_ptr;
            if (parent_node) {
               parent_node->children.insert(parent_node->children.begin() + row + i, &item);
            } else {
               this->_data.insert(this->_data.begin() + row + i, &item);
            }
            item_ptr.release();
            item.form = dropped_stubs[i];
            item.cached.editor_id = QString::fromStdString(item.form->editorID);
         }
         this->endInsertRows();
         return true;
      }
   #pragma endregion
   #pragma region Drag-moving nodes within our tree
      QMimeData* RegionObjectsModel::_get_node_drag_data(const QModelIndexList& indices) const {
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
            // The default design for QAbstractItemModel is to serialize all itemData for 
            // a dragged node into the stream. "Moving" an item actually involves removing 
            // it from the tree and then inserting a new copy somewhere else.
            // 
            // This is dumb and wasteful, it only works if all of the node data can be 
            // encoded as Qt item data, AFAIK it doesn't account for child/descendant nodes, 
            // and for most of the models we make, it's not possible for a variety of other 
            // reasons.
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
            stream << this->_drag_and_drop.track(*const_cast<Object*>(node));
         }
         //
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

         QByteArray  data = mime.data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((RegionObjectsModel*)this_pointer != this)
               return false;
         }

         const auto* parent_node = _node_for_qmi(parent_qmi);
         if (!parent_node)
            return true;

         //
         // Don't allow dragging any node into itself or its own descendants.
         //
         std::vector<Object*> dragged_nodes;
         while (!stream.atEnd()) {
            Object* node = nullptr;
            {
               DragDropTracking::uid_t id;
               stream >> id;
               node = this->_drag_and_drop.get_by_id(id);
            }
            if (!node)
               continue;

            if (node == parent_node)
               return false;
            if (node->contains(*parent_node))
               return false;
         }
         return true;
      }
      bool RegionObjectsModel::_drop_nodes(const QMimeData& mime, Qt::DropAction action, const QModelIndex& dst_parent_qmi, Object* dst_parent_node, int row) {
         QByteArray  data = mime.data(mime_type);
         QDataStream stream(&data, QIODevice::ReadOnly);
         {  // Verify that this is an internal move.
            std::intptr_t this_pointer;
            stream >> this_pointer;
            if ((RegionObjectsModel*)this_pointer != this)
               return false;
         }
         std::vector<Object*> nodes;
         while (!stream.atEnd()) {
            DragDropTracking::uid_t id;
            stream >> id;
            auto* node = this->_drag_and_drop.get_by_id(id);
            if (node)
               nodes.push_back(node);
         }
         if (!nodes.size())
            return false;

         auto* dst_siblings = &this->_data;
         if (dst_parent_node) {
            dst_siblings = &dst_parent_node->children;
         }
         for (auto it = nodes.rbegin(); it != nodes.rend(); ++it) {
            Object& node = **it;

            auto*       src_siblings = &this->_data;
            QModelIndex src_parent_qmi;
            if (node.parent) {
               src_siblings   = &node.parent->children;
               src_parent_qmi = _qmi_for_node(*node.parent);
            }
            int src_i = -1;
            for (size_t i = 0; i < src_siblings->size(); ++i) {
               if ((*src_siblings)[i] == &node) {
                  src_i = i;
                  break;
               }
            }
            assert(src_i >= 0);

            this->beginMoveRows(src_parent_qmi, src_i, src_i, dst_parent_qmi, row);
            dst_siblings->insert(dst_siblings->begin() + row, &node);
            src_siblings->erase(src_siblings->begin() + src_i);
            this->endMoveRows();
         }
         return true;
      }

      #pragma region DragDropTracking
         RegionObjectsModel::DragDropTracking::uid_t RegionObjectsModel::DragDropTracking::track(Object& node) {
            for (const auto& pair : this->nodes)
               if (pair.second == &node)
                  return pair.first;
            auto id = this->next_id;
            this->next_id++;
            this->nodes[id] = &node;
            return id;
         }
         void RegionObjectsModel::DragDropTracking::untrack(Object& node) {
            auto& map = this->nodes;
            auto  it  = std::find_if(map.begin(), map.end(), [&node](const auto& pair) {
               return pair.second == &node;
            });
            if (it != map.end())
               map.erase(it);
         }
         void RegionObjectsModel::DragDropTracking::clear() {
            this->nodes.clear();
         }
         RegionObjectsModel::Object* RegionObjectsModel::DragDropTracking::get_by_id(uid_t id) {
            auto& map = this->nodes;
            auto  it  = map.find(id);
            if (it != map.end())
               return it->second;
            return nullptr;
         }
         void RegionObjectsModel::DragDropTracking::on_node_destroyed(Object& node) {
            [this](this auto&& recurse, Object& node) -> void {
               this->untrack(node);
               for (auto* child : node.children)
                  recurse(*child);
            }(node);
         }
      #pragma endregion
   #pragma endregion
#pragma endregion