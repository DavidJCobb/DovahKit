#include "./DKRefsInCellModel.h"
#include <cassert>
#include <vector>
#include "dovah/form_stub.h"
#include "dovah/form_stub_helpers.h"
#include "editor/core.h"

#define TEST_FILTERING_INVARIANTS _DEBUG

#pragma region Item
DKRefsInCellModel::Item::Item(dovah::form_stub* stub) {
   this->stub = stub;
   this->updateFromStub();
}
void DKRefsInCellModel::Item::updateFromStub() {
   dovah::form_stub* stub = this->stub;
   dovah::form_stub* base = nullptr;
   if (!stub) {
      this->editor_ids  = {};
      this->cached_text = "NONE";
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

   for (auto* item : this->filtered_out)
      delete item;
   this->filtered_out.clear();

   this->parent_cell = nullptr;
}

bool DKRefsInCellModel::_item_matches_filter(const Item& item) const {
   if (this->filter_string.isEmpty())
      return true;
   return item.cached_text.contains(this->filter_string, Qt::CaseInsensitive);
}
bool DKRefsInCellModel::_stub_allowed_in_model(const dovah::form_stub& stub) const {
   if (!dovah::form_type_info::form_type_is_reference(stub.formType))
      return false;
   if (stub.editorID.empty())
      return false;
   return true;
}

void DKRefsInCellModel::_clear() {
   for (auto* item : this->filtered_out)
      delete item;
   this->filtered_out.clear();

   if (this->children.empty())
      return;

   this->beginResetModel();

   for (auto* item : this->children)
      delete item;
   this->children.clear();

   this->endResetModel();
}

QVector<DKRefsInCellModel::Item*> DKRefsInCellModel::_handle_newly_concealed_by_filter() {

   // Control whether we batch calls to beginRemoveRows, in order to reduce UI updates and 
   // similar signal responses. Will lead to increased overhead within this function.
   constexpr const bool batch_removals = true;

   if (this->children.empty())
      return {};

   QVector<Item*> newly_removed;
   if constexpr (batch_removals) {
      struct range {
         size_t start = 0;
         size_t size  = 0;
      };
      std::vector<range> ranges;

      for (size_t i = 0; i < this->children.size(); ++i) {
         auto* item = this->children[i];
         if (this->_item_matches_filter(*item))
            continue;

         newly_removed.push_back(item);

         if (!ranges.empty()) {
            auto& last = ranges.back();
            if (last.start + last.size == i) {
               ++last.size;
               continue;
            }
         }
         ranges.push_back({
            .start = i,
            .size  = 1,
         });
      }

      size_t removed = 0;
      for (auto& range : ranges) {
         size_t start = range.start - removed;
         size_t count = range.size;

         this->beginRemoveRows({}, start, start + count - 1);
         this->children.remove(start, count);
         this->endRemoveRows();

         removed += count;
      }
   } else {
      size_t size = this->children.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = this->children[i];
         if (this->_item_matches_filter(*item))
            continue;

         newly_removed.push_back(item);
         this->beginRemoveRows({}, i, i);
         this->children.remove(i);
         this->endRemoveRows();

         --i;
         --size;
      }
   }

   return newly_removed;
}
void DKRefsInCellModel::_handle_newly_revealed_by_filter() {

   // Control whether we batch calls to beginInsertRows, in order to reduce UI updates and 
   // similar signal responses. Will lead to increased overhead within this function.
   constexpr const bool batch_insertions = true;

   if (this->filtered_out.empty())
      return;
   
   if constexpr (!batch_insertions) {
      size_t size = this->filtered_out.size();
      for (size_t i = 0; i < size; ++i) {
         auto* item = this->filtered_out[i];
         if (!this->_item_matches_filter(*item))
            continue;

         this->_insert_sorted_item(item);
         this->filtered_out.remove(i);
      }
   } else {
      //
      // This is an algorithm to selectively mix two lists together given the following 
      // initial conditions and goals:
      // 
      //  - Elements will be transferred from the source list into the destination list 
      //    if they meet some condition.
      // 
      //  - The source list is unsorted.
      // 
      //  - The destination list is sorted, and must remain so.
      // 
      //  - We wish to avoid having to re-sort the entire destination list; we wish to 
      //    transfer elements right to their final destinations.
      // 
      //  - We wish to batch insertions: there is some processing that we must do when 
      //    a single item or a contiguous range of items are added to the destination 
      //    list. A group of items is here considered a "contiguous range" if, once 
      //    placed within the destination range, they are contiguous, even if the group 
      //    was disjoint within the source list.
      // 
      //    That is: given the following source and destination lists,
      // 
      //       src: [0, 1, 2, 3]
      //       dst: [A, B, C, D]
      // 
      //    The following result would involve the insertion of a contiguous range:
      // 
      //       [0, 1, A, D, B, 2, 3, C]
      // 
      // To complete this task, we start by building `mixed_indices`, a list of indices 
      // within both lists. We co-opt the most significant bit in each index in order to 
      // mark which list it belongs to (the bit is set for indices in the source list). 
      // The `mixed_indices` list starts holding just the destination indices:
      // 
      //    [0, 1, 2, 3]
      // 
      // We loop through the source list, skipping any items we don't intend to transfer. 
      // Given an item we wish to transfer, we loop through `mixed_indices` to access the 
      // sorted elements and compare them against the to-be-transferred item, to see where 
      // the to-be-transferred item should be placed. Why do we use `mixed_indices`? So 
      // that we can compare the to-be-transferred item to other, previously seen, to-be-
      // transferred items.
      // 
      // Continuing our example from above: after we see A, decide to transfer it, and 
      // determine its final place in the destination list, `mixed_indices` is:
      // 
      //    [0, 1, A, 2, 3]
      // 
      // And now, when we evaluate B, we can compare it not only to the destination list 
      // items, but also to A:
      // 
      //    [0, 1, A, B, 2, 3]
      // 
      // Thus, also, to C:
      // 
      //    [0, 1, A, B, 2, 3, C]
      // 
      // And finally, D can be compared, so we can tell that it belongs between A and B, 
      // not merely between 1 and 2:
      // 
      //    [0, 1, A, D, B, 2, 3, C]
      // 
      // Once `mixed_indices` has been fully computed as per the above, it's trivial to 
      // loop over it and check for multiple source-list indices in a row (e.g. the span 
      // of [A, D, B]) as we go, transferring those source-list items all at once.
      //
      // -------------------------------------------------------------------------------
      //
      // For our purposes, `this->filtered_out` is the source list, comprising all model 
      // nodes that were concealed by the previous filter; and `this->children` is the 
      // destination list, comprising all model nodes that should be visible.
      //

      constexpr const size_t is_source_list_index = (size_t)1 << (sizeof(size_t) * 8 - 1);

      std::vector<int> mixed_indices;
      size_t count_revealed = 0;
      {
         {
            size_t size = this->children.size();
            mixed_indices.resize(size);
            for (int i = 0; i < size; ++i)
               mixed_indices[i] = i;
         }

         for (int i = 0; i < this->filtered_out.size(); ++i) {
            auto* item = this->filtered_out[i];

            if (!this->_item_matches_filter(*item))
               continue;
            ++count_revealed;

            bool found = false;
            for (size_t j = 0; j < mixed_indices.size(); ++j) {
               Item* element;
               auto  index = mixed_indices[j];
               if (index & is_source_list_index) {
                  element = this->filtered_out[(index & ~is_source_list_index)];
               } else {
                  element = this->children[index];
               }

               if (item->cached_text.compare(element->cached_text, Qt::CaseInsensitive) > 0) {
                  mixed_indices.insert(mixed_indices.begin() + j, i | is_source_list_index);
                  found = true;
                  break;
               }
            }
            if (!found)
               mixed_indices.push_back(i | is_source_list_index);
         }
      }

      //
      // Now that `mixed_indices` is constructed, we can go ahead and begin transferring 
      // items into the destination list.
      //

      if (count_revealed) {
         size_t i = 0;
         do {
            if (!(mixed_indices[i] & is_source_list_index))
               continue;

            size_t count = 1;
            for (size_t j = i + 1; j < mixed_indices.size(); ++j) {
               if (!(mixed_indices[j] & is_source_list_index))
                  break;
               ++count;
            }

            this->beginInsertRows({}, i, i + count - 1);
            for (size_t n = 0; n < count; ++n) {
               size_t from = mixed_indices[i + n] & ~is_source_list_index;
               this->children.insert(i + n, this->filtered_out[from]);
               this->filtered_out[from] = nullptr;
            }
            this->endInsertRows();

            i += count;
         } while (i < mixed_indices.size());

         //
         // Now that items were moved to the destination list, remove them from the source 
         // list:
         //

         if (count_revealed == this->filtered_out.size()) {
            this->filtered_out.clear();
         } else {
            auto& src = this->filtered_out;
            src.erase(
               std::remove_if(
                  src.begin(),
                  src.end(),
                  [](auto* v) {
                     return v == nullptr;
                  }
               ),
               src.end()
            );
         }
      }
   }
}
//
void DKRefsInCellModel::_filter(bool filter_made_more_specific) {

   // Control whether we batch calls to beginInsertRows, in order to reduce UI updates and 
   // similar signal responses. Will lead to increased overhead within this function.
   constexpr const bool batch_insertions = true;

   if (this->filtered_out.empty()) {
      if (this->filter_string.isEmpty())
         return;
      if (this->children.empty())
         return;
   }
   if (filter_made_more_specific) {
      if (this->children.empty())
         //
         // If the filter was made more specific, but we've already filtered 
         // everything out, then we're not exactly going to be filtering any 
         // of those items back *in*, now, are we? There's no work to do.
         //
         return;
   }

   #if TEST_FILTERING_INVARIANTS
   QVector<Item*> previous_filter = this->filtered_out;
   QVector<Item*> previous_shown = this->children;
   #endif

   QVector<Item*> newly_removed = this->_handle_newly_concealed_by_filter();

   this->_handle_newly_revealed_by_filter();

   // Now that we've properly handled all potentially revealed items, take the 
   // newly filtered items and add them into the "filtered out" list.
   this->filtered_out.reserve(this->filtered_out.size() + newly_removed.size());
   for (auto* item : newly_removed)
      this->filtered_out.push_back(item);


   #if TEST_FILTERING_INVARIANTS
   for (auto* a : this->children)
      for (auto* b : this->filtered_out)
         assert(a != b && "Assert: No items should be both filtered and not filtered.");

   for (auto* item : previous_filter)
      assert(this->children.contains(item) || this->filtered_out.contains(item) && "Assert: No items should've been lost.");
   for (auto* item : previous_shown)
      assert(this->children.contains(item) || this->filtered_out.contains(item) && "Assert: No items should've been lost.");
   #endif
}
void DKRefsInCellModel::_insert_sorted_item(Item* item) {
   if (!this->_item_matches_filter(*item)) {
      this->filtered_out.push_back(item);
      return;
   }
   size_t at;
   for (at = 0; at < this->children.size(); ++at) {
      auto* prior = this->children[at];
      if (item->is_prepended) {
         if (!prior->is_prepended)
            break;
      } else {
         if (prior->is_prepended)
            continue;
      }
      if (prior->cached_text.compare(item->cached_text, Qt::CaseInsensitive) < 0)
         break;
   }

   this->beginInsertRows({}, at, at);
   this->children.insert(at, item);
   this->endInsertRows();
}
void DKRefsInCellModel::_insert_sorted_stub(dovah::form_stub* ref, bool prepended) {
   auto* item = new Item(ref);
   item->is_prepended = prepended;
   this->_insert_sorted_item(item);
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
      return a.cached_text.compare(b.cached_text, Qt::CaseInsensitive) < 0;
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

void DKRefsInCellModel::addPrependedRef(dovah::form_stub* ref) {
   for (auto* item : this->children) {
      if (item->stub == ref) {
         if (item->is_prepended)
            return;
         item->is_prepended = true;
         this->_sort();
         return;
      }
   }
   this->_insert_sorted_stub(ref, true);
}
void DKRefsInCellModel::removePrependedRef(dovah::form_stub* ref) {
   bool is_in_cell = this->parent_cell && ref && ref->get_parent_form() == this->parent_cell;

   for (size_t i = 0; i < this->children.size(); ++i) {
      auto* item = this->children[i];

      if (!item->is_prepended)
         //
         // We've passed the prepended items.
         //
         return;

      if (item->stub == ref) {

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

      if (!item->stub || item->stub->get_parent_form() != this->parent_cell) {
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
         this->children.resize(start);
         this->endRemoveRows();
      }
      size = this->children.size();
   }

   if (!cell) {
      return;
   }

   QVector<Item*> working;
   dovah::form_stub_helpers::for_each_child_form(cell, [this, &working](dovah::form_stub* ref) -> bool {
      if (!_stub_allowed_in_model(*ref))
         return false;

      for (auto* prepended : this->children)
         if (prepended->stub == ref)
            return false;

      auto* item = new Item(ref);
      working.push_back(item);

      return false;
   });
   if (!working.empty()) {
      this->beginInsertRows({}, size, size + working.size() - 1);
      this->children.reserve(size + working.size());
      for (auto* item : working)
         this->children.push_back(item);
      this->endInsertRows();
      this->_sort();
   }
}

QString DKRefsInCellModel::filterString() const {
   return this->filter_string;
}
void DKRefsInCellModel::setFilterString(QString s) {
   if (this->filter_string == s)
      return;
   bool more_specific = s.contains(this->filter_string);
   this->filter_string = s;
   this->_filter(more_specific);
}

dovah::form_stub* DKRefsInCellModel::ref(QModelIndex qmi) const {
   if (!qmi.isValid() || qmi.model() != this)
      return nullptr;
   if (qmi.row() >= this->children.size())
      return nullptr;
   return this->children[qmi.row()]->stub;
}

#pragma region Editor core hooks
void DKRefsInCellModel::formCreated(dovah::form_stub* stub) {
   if (!stub) // this signal should've used a ref...
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;
   if (!this->parent_cell)
      return;
   if (stub->get_parent_form() != this->parent_cell)
      return;
   this->_insert_sorted_stub(stub, false);
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
   if (!stub) // this signal should've used a ref...
      return;
   if (!dovah::form_type_info::form_type_is_reference(stub->formType))
      return;

   bool in_our_cell = this->parent_cell && stub->get_parent_form() == this->parent_cell;

   auto& list = this->children;
   auto  size = list.size();
   for (size_t i = 0; i < size; ++i) {
      auto* item = list[i];
      if (item->stub != stub)
         continue;

      if ((!item->is_prepended && !in_our_cell) || !_stub_allowed_in_model(*stub)) {
         //
         // Ref was reparented and is no longer in the cell we're interested in, OR 
         // the stub had its editor ID cleared or was otherwise made unsuitable.
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

   //
   // The stub is not currently in this model.
   //

   if (in_our_cell && _stub_allowed_in_model(*stub)) {
      //
      // Ref was reparented into the cell we're interested in.
      //
      this->_insert_sorted_stub(stub, false);
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
      case FormStubRole:
         return QVariant::fromValue<void*>(item->stub);
   }
   return {};
}
QVariant DKRefsInCellModel::headerData(int section, Qt::Orientation orientation, int role) const {
   return {};
}
#pragma endregion