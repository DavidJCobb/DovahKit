#include "./DKRefsInCellModel.h"
#include <cassert>
#include <vector>
#include "dovah/form_stub.h"
#include "dovah/form_stubs/helpers/for_each_child_form.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/core.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/papyrus/core.h"

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
   this->no_editor_id = this->editor_ids.ref.isEmpty();

   QString me = this->editor_ids.ref;
   if (me.isEmpty())
      me = editor_helpers::form_identifiers_to_string(stub);

   base = dovah::form_stub_helpers::get_base_form(*stub);
   if (base) {
      this->editor_ids.base = base->get_editor_id();
      this->cached_text = QString("%1 (%2)").arg(me).arg(this->editor_ids.base);
   } else {
      this->editor_ids.base.clear();
      this->cached_text = QString("%1 (NONE)").arg(me);
   }
}

bool DKRefsInCellModel::Item::sortAbove(const Item& other) const {
   if (this->is_prepended != other.is_prepended)
      return this->is_prepended;
   if (this->no_editor_id != other.no_editor_id)
      return !this->no_editor_id;
   return this->cached_text.compare(other.cached_text, Qt::CaseInsensitive) < 0;
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
   if (item.is_prepended) // never filter force-prepended items out
      return true;
   if (this->filter_form_type != dovah::form_type::none && this->filter_form_type != dovah::form_type::reference) {
      if (item.stub->form_type != this->filter_form_type)
         return false;
   }
   if (!this->filter_string.isEmpty())
      if (!item.cached_text.contains(this->filter_string, Qt::CaseInsensitive))
         return false;
   if (!this->required_scriptname.empty()) {
      const auto& papyrus = dovahkit::subsystems::papyrus::core::get();
      if (!papyrus.form_has_script_attached(*item.stub, this->required_scriptname))
         return false;
   }
   return true;
}
bool DKRefsInCellModel::_stub_allowed_in_model(const dovah::form_stub& stub) const {
   if (!dovah::form_type_is_reference(stub.form_type))
      return false;
   /*//
   if (stub.editorID.empty())
      return false;
   //*/
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

      std::vector<size_t> mixed_indices;
      size_t count_revealed = 0;
      {
         {
            size_t size = this->children.size();
            mixed_indices.resize(size);
            for (size_t i = 0; i < size; ++i)
               mixed_indices[i] = i;
         }

         for (int i = 0; i < this->filtered_out.size(); ++i) {
            auto* item = this->filtered_out[i];

            if (!this->_item_matches_filter(*item))
               continue;
            ++count_revealed;

            //
            // Now we need to figure out where to put `item` in the list.
            //
            bool found = false;
            for (size_t j = 0; j < mixed_indices.size(); ++j) {
               Item* element;
               auto  index = mixed_indices[j];
               if (index & is_source_list_index) {
                  element = this->filtered_out[(index & ~is_source_list_index)];
               } else {
                  element = this->children[index];
               }

               if (item->sortAbove(*element)) {
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
            //
            // Start by finding the next contiguous range of items to move.
            //
            if (!(mixed_indices[i] & is_source_list_index)) {
               ++i;
               continue;
            }
            // mixed_indices[i] is the start of the range.

            size_t count = 1;
            for (size_t j = i + 1; j < mixed_indices.size(); ++j) {
               if (!(mixed_indices[j] & is_source_list_index))
                  break;
               ++count;
            }
            // count is the size of the range.

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
      if (this->filter_string.isEmpty() && this->required_scriptname.empty())
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
      if (!prior->sortAbove(*item))
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
      return a.sortAbove(b);
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
      int row_after = -1;
      if (row_prior >= 0 && row_prior < size) {
         for (size_t i = 0; i < size; ++i) {
            if (mapping[i] == row_prior) {
               row_after = i;
               break;
            }
         }
      }
      map_to.push_back(this->index(row_after, pqmi.column(), {}));
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
   dovah::form_stub_helpers::for_each_child_form(*cell, [this, &working](dovah::form_stub& ref) -> bool {
      if (!_stub_allowed_in_model(ref))
         return false;

      for (auto* prepended : this->children)
         if (prepended->stub == &ref)
            return false;

      auto* item = new Item(&ref);
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

void DKRefsInCellModel::setRequiredScriptname(QString s) {
   std::string desired = s.toUtf8().toStdString();
   this->setRequiredScriptname(std::string_view(desired));
}
void DKRefsInCellModel::setRequiredScriptname(std::string_view desired) {
   if (dovah::papyrus::helpers::name_equals(desired, this->required_scriptname))
      return;
   this->required_scriptname = desired;
   this->_filter();
}

void DKRefsInCellModel::setRequiredFormType(dovah::form_type ft) {
   if (ft == dovah::form_type::reference)
      ft = dovah::form_type::none;
   if (ft == this->requiredFormType())
      return;
   this->filter_form_type = ft;
   this->_filter();
}

dovah::form_stub* DKRefsInCellModel::ref(QModelIndex qmi) const {
   if (!qmi.isValid() || qmi.model() != this)
      return nullptr;
   if (qmi.row() >= this->children.size())
      return nullptr;
   return this->children[qmi.row()]->stub;
}

bool DKRefsInCellModel::refMatchesHardFilters(dovah::form_stub* ref) const {
   //
   // Prepended items always match.
   //
   for (auto& item : this->children) {
      if (!item->is_prepended)
         break;
      if (ref == item->stub)
         return true;
   }
   //
   // Otherwise, apply the usual constraints EXCEPT for the filter string. The 
   // filter string is used to have the user narrow down matching refs, not to 
   // determine what refs are legal.
   //
   if (!ref)
      return false;
   if (this->filter_form_type != dovah::form_type::none && this->filter_form_type != dovah::form_type::reference) {
      if (ref->form_type != this->filter_form_type)
         return false;
   }
   if (!this->required_scriptname.empty()) {
      const auto& papyrus = dovahkit::subsystems::papyrus::core::get();
      if (!papyrus.form_has_script_attached(*ref, this->required_scriptname))
         return false;
   }
   return true;
}

#pragma region Editor core hooks
void DKRefsInCellModel::formCreated(dovah::form_stub* stub) {
   if (!stub) // this signal should've used a ref...
      return;
   if (!dovah::form_type_is_reference(stub->form_type))
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

   if (!dovah::form_type_is_reference(stub->form_type))
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
   if (!dovah::form_type_is_reference(stub->form_type))
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
      return {};
   return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
}
QVariant DKRefsInCellModel::data(const QModelIndex& index, int role) const {
   if (!index.isValid() || index.model() != this)
      return {};
   auto item = (const Item*)index.internalPointer();
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