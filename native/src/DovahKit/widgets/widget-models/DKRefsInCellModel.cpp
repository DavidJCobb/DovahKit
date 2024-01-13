#include "./DKRefsInCellModel.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_helpers.h"
#include "editor/core.h"

#pragma region Item
DKRefsInCellModel::Item::Item(dovah::form_stub* stub) {
   this->stub = stub;
   this->updateFromStub();
}
void DKRefsInCellModel::Item::updateFromStub() {
   dovah::form_stub* stub = this->stub;
   dovah::form_stub* base = nullptr;
   if (!stub) {
      this->editor_ids = {};
      return;
   }

   this->editor_ids.ref = stub->get_editor_id();

   base = dovah::form_stub_helpers::get_base_form(stub);
   if (base) {
      this->editor_ids.base = base->get_editor_id();
      this->cached_text = QString("%1 (%2)").arg(this->editor_ids.ref).arg(this->editor_ids.base);
   } else {
      this->editor_ids.base.clear();
      this->cached_text = QString("%1 (NONE)").arg(this->editor_ids.ref);
   }
}
#pragma endregion

DKRefsInCellModel::DKRefsInCellModel(QObject* parent) : QAbstractListModel(parent) {
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::formCreated,            this, &DKRefsInCellModel::formCreated);
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent,   this, &DKRefsInCellModel::formDeletionImminent);
   QObject::connect(&editor, &DovahKitCore::formModified,           this, &DKRefsInCellModel::formModified);
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent,    this, &DKRefsInCellModel::_clear);
}

DKRefsInCellModel::~DKRefsInCellModel() {
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->parent_cell = nullptr;
}

void DKRefsInCellModel::_clear() {
   if (this->children.empty())
      return;
   this->beginResetModel();
   for (auto* item : this->children)
      delete item;
   this->children.clear();
   this->endResetModel();
}
void DKRefsInCellModel::_insert_sorted_stub(dovah::form_stub& ref, bool prepended) {
   Item   to_add(&ref);
   size_t at;
   for (at = 0; at < this->children.size(); ++at) {
      auto* prior = this->children[at];
      if (prepended) {
         if (!prior->is_prepended)
            break;
      } else {
         if (prior->is_prepended)
            continue;
      }
      if (prior->cached_text.compare(to_add.cached_text) < 0)
         break;
   }

   this->beginInsertRows({}, at, at);
   this->children.insert(at, new Item(std::move(to_add)));
   this->endInsertRows();
}
void DKRefsInCellModel::_sort() {
   emit layoutAboutToBeChanged({}, QAbstractItemModel::LayoutChangeHint::VerticalSortHint);

   size_t size = this->children.size();

   //
   // Figure out our sorting. Don't sort the list directly; rather, make a list of all its 
   // indices and sort that. You'll see why.
   //
   QVector<size_t> mapping;
   mapping.resize(size);
   for (size_t i = 0; i < size; ++i)
      mapping[i] = i;
   //
   std::stable_sort(mapping.begin(), mapping.end(), [this](const size_t i, const size_t j) {
      const auto& a = *this->children[i];
      const auto& b = *this->children[j];

      if (a.is_prepended) {
         if (!b.is_prepended)
            return true;
      } else if (b.is_prepended) {
         return false;
      }
      return a.cached_text.compare(b.cached_text) < 0;
   });

   //
   // Prepare to update all extant QPersistentModelIndexes pointing to our model. This is 
   // why we needed to sort the indices: so we can tell, given an "old" index, where the 
   // "new" index is.
   //
   QModelIndexList map_from = this->persistentIndexList();
   QModelIndexList map_to;
   for (auto& pqmi : map_from) {
      int row_prior = pqmi.row();
      int row_after;
      if (row_prior > size) {
         row_after = -1;
      } else {
         row_after = mapping[row_prior];
      }
      map_to.push_back(this->index(row_after, 0, {}));
   }

   //
   // Actually sort our list.
   //
   QVector<Item*> sorted;
   sorted.resize(size);
   for (size_t i = 0; i < size; ++i) {
      sorted[i] = this->children[mapping[i]];
   }
   std::swap(this->children, sorted);

   //
   // Update the QPMIs now.
   //
   this->changePersistentIndexList(map_from, map_to);

   emit layoutChanged();
}

void DKRefsInCellModel::addPrependedRef(dovah::form_stub& ref) {
   for (auto* item : this->children) {
      if (item->stub == &ref) {
         if (item->is_prepended)
            return;
         item->is_prepended = true;
         this->_sort();
         return;
      }
   }
   this->_insert_sorted_stub(ref, true);
}
void DKRefsInCellModel::removePrependedRef(dovah::form_stub& ref) {
   bool is_in_cell = this->parent_cell && ref.get_parent_form() == this->parent_cell;

   for (size_t i = 0; i < this->children.size(); ++i) {
      auto* item = this->children[i];

      if (!item->is_prepended)
         //
         // We've passed the prepended items.
         //
         return;

      if (item->stub == &ref) {

         if (is_in_cell) {
            if (!item->is_prepended)
               return;
            item->is_prepended = false;
            this->_sort();
         } else {
            this->beginRemoveRows({}, i, i);
            this->children.remove(i);
            delete item;
            this->endRemoveRows();
         }

         return;
      }
   }
}
void DKRefsInCellModel::clearAllPrependedRefs() {
   if (this->children.empty() || this->children[0]->is_prepended == false)
      return;

   size_t size = this->children.size();

   if (!this->parent_cell) {
      //
      // The only items in the dropdown are forcibly-prepended items, so we're 
      // effectively just emptying the whole dropdown. Simple and easy.
      //
      this->beginRemoveRows({}, 0, size);
      for (auto* item : this->children)
         delete item;
      this->children.clear();
      this->endRemoveRows();
      return;
   }

   bool any_moved = false;

   size_t i;
   for (i = 0; i < size; ++i) {
      auto* item = this->children[i];
      if (!item->is_prepended)
         break;

      if (!item->stub)
         continue;
      if (item->stub->get_parent_form() != this->parent_cell) {
         this->beginRemoveRows({}, i, i);
         this->children.remove(i);
         delete item;
         this->endRemoveRows();

         --i;
         --size;
      } else {
         item->is_prepended = false;

         any_moved = true;
      }
   }

   if (any_moved) {
      this->_sort();
   }
}

dovah::form_stub* DKRefsInCellModel::parentCell() const {
   return this->parent_cell;
}
void DKRefsInCellModel::setParentCell(dovah::form_stub* cell) {
   if (cell == this->parent_cell)
      return;
   this->parent_cell = cell;

   size_t size = this->children.size();

   //
   // Clear non-prepended refs:
   //
   {
      size_t start;
      for (start = 0; start < size; ++start) {
         if (!this->children[start]->is_prepended)
            break;
      }
      if (start < size) {
         this->beginRemoveRows({}, start, size - 1);
         for (size_t i = start; i < size; ++i)
            delete this->children[i];
         this->children.clear();
         this->endRemoveRows();
      }
   }

   if (!cell) {
      return;
   }

   QVector<Item*> working;
   dovah::form_stub_helpers::for_each_child_form(cell, [this, &working](dovah::form_stub* ref) -> bool {
      if (!dovah::form_type_info::form_type_is_reference(ref->formType))
         return false;

      for (auto* prepended : this->children)
         if (prepended->stub == ref)
            return false;

      auto* item = new Item(ref);
      working.push_back(item);

      return false;
   });
   this->beginInsertRows({}, size, size + working.size() - 1);
   this->children.reserve(size + working.size());
   for (auto* item : working)
      this->children.push_back(item);
   this->endInsertRows();
   this->_sort();
}


#pragma region Editor core hooks
void DKRefsInCellModel::formCreated(dovah::form_stub* stub) {
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;
   if (!this->parent_cell)
      return;
   if (stub->get_parent_form() != this->parent_cell)
      return;
   this->_insert_sorted_stub(*stub, false);
}
void DKRefsInCellModel::formDeletionImminent(const dovah::form_stub* stub, bool is_just_flagged) {
   if (!stub) // this signal should've used a ref...
      return;
   if (stub == this->parent_cell) {
      this->setParentCell(nullptr);
      return;
   }

   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;
   for (size_t i = 0; i < this->children.size(); ++i) {
      auto* item = this->children[i];
      if (item->stub == stub) {
         this->beginRemoveRows({}, i, i);
         this->children.remove(i);
         delete item;
         this->endRemoveRows();
         return;
      }
   }
}
void DKRefsInCellModel::formModified(dovah::form_stub* stub) {
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;

   bool in_our_cell = this->parent_cell && stub->get_parent_form() == this->parent_cell;

   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub != stub)
         continue;

      if (!item->is_prepended && !in_our_cell) {
         //
         // Ref was reparented and is no longer in the cell we're interested in.
         //
         this->beginRemoveRows({}, i, i);
         this->children.remove(i);
         delete item;
         this->endRemoveRows();
         return;
      }

      item->updateFromStub();
      auto qmi = this->index(i, 0, {});
      emit dataChanged(qmi, qmi);
      return;
   }

   if (in_our_cell) {
      //
      // Ref was reparented into the cell we're interested in.
      //
      this->_insert_sorted_stub(*stub, false);
   }
}
#pragma endregion

#pragma region QAbstractItemModel overrides
QModelIndex DKRefsInCellModel::index(int row, int column, const QModelIndex& parent) const {
   if (!this->hasIndex(row, column, parent))
      return {};
   Item* childItem = this->children.value(row);
   if (childItem)
      return this->createIndex(row, column, childItem);
   return {};
}
QModelIndex DKRefsInCellModel::parent(const QModelIndex& index) const {
   return {};
}
int DKRefsInCellModel::rowCount(const QModelIndex& parent) const {
   if (parent.column() > 0)
      return 0;
   return this->children.size();
}
int DKRefsInCellModel::columnCount(const QModelIndex& item) const {
   return 1;
}
Qt::ItemFlags DKRefsInCellModel::flags(const QModelIndex& index) const {
   if (!index.isValid())
      return Qt::ItemIsDropEnabled;
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKRefsInCellModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid() || index.model() != this)
      return {};
   auto item = (Item*)index.internalPointer();
   switch (role) {
      case Qt::DisplayRole:
      case Qt::ToolTipRole:
         return item->cached_text;
      case ForcedPrependedRole:
         return item->is_prepended;
   }
   return {};
}
QVariant DKRefsInCellModel::headerData(int section, Qt::Orientation orientation, int role) const {
   return {};
}
#pragma endregion