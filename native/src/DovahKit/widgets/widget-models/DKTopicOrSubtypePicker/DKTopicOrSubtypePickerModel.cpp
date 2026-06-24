#include "./DKTopicOrSubtypePickerModel.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/form_info_cache/cached_data/by_form_type/topic.h"
#include "editor/subsystems/form_info_cache/core.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/localize/dialogue_topic_subtype.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

namespace {
   constexpr int max_fill_tick_duration = 33; // milliseconds
   #if _DEBUG
      constexpr bool do_fill_diagnostics = true;
      constexpr int  estimated_items_possible_in_one_tick = 1000; // budget for less-async fills. per control, and some windows have several FormPickers, so keep it low.
   #else
      constexpr bool do_fill_diagnostics = false;
      constexpr int  estimated_items_possible_in_one_tick = 2000; // budget for less-async fills. per control, and some windows have several FormPickers, so keep it low.
   #endif

   constexpr const auto topic_subtypes_to_exclude = std::array{
      (uint32_t)'SCEN',
   };

   constexpr const auto permitted_subtypes = []() {
      std::array<
         const dovah::dialogue::topic_subtype*,
         dovah::dialogue::all_topic_subtypes.size() - topic_subtypes_to_exclude.size()
      > out = {};
      size_t i = 0;
      for (auto& info : dovah::dialogue::all_topic_subtypes) {
         bool allowed = true;
         for (const auto signature : topic_subtypes_to_exclude) {
            if (signature == info.signature) {
               allowed = false;
               break;
            }
         }
         if (allowed) {
            out[i] = &info;
            ++i;
         }
      }
      return out;
   }();
}

namespace ui::impl::DKTopicOrSubtypePicker {
   #pragma region Model
      Model::Model(QObject* parent) : QAbstractItemModel(parent) {
         //
         // Build the subtype list in advance. Strictly speaking, this should be 
         // constexpr or in a "datastore" object, but the use of translatable 
         // QStrings locks us out of the first option, and this widget won't 
         // appear in large enough numbers to justify the second option; building 
         // and sorting this list is *far* faster than chugging through every 
         // loaded form in Skyrim.esm.
         //
         this->_subtypes.reserve(permitted_subtypes.size());
         for (const auto* info : permitted_subtypes) {
            auto& dst = this->_subtypes.emplace_back();
            dst.name      = tr("(%1)").arg(editor::localize::dialogue_topic_subtype(*info));
            dst.signature = info->signature;
         }
         std::sort(
            this->_subtypes.begin(),
            this->_subtypes.end(),
            [](const cached_subtype& a, const cached_subtype& b) -> bool {
               return a.name.compare(b.name, Qt::CaseInsensitive) < 0;
            }
         );

         //
         // Form datastore-related functionality.
         //
         QObject::connect(&this->_ongoing_fill.timer, &QTimer::timeout, this, [this]() {
            this->fetchMore({});
         });
         //
         auto& source = shared_datastore::get();
         QObject::connect(&source, &shared_datastore::rowsInserted, this, [this](size_t first, size_t last) {
            auto& source   = shared_datastore::get();
            auto& sorted   = this->_forms;
            auto& unsorted = this->_ongoing_fill.unsorted;
            //
            auto& of = this->_ongoing_fill;
            
            bool is_during_fill = false;
            switch (of.stage) {
               case _fill_stage::currently_filling:
               case _fill_stage::currently_sorting:
                  is_during_fill = true;
                  break;
            }
            const auto& params = is_during_fill ? of.params : this->_last_completed_fill_params;

            switch (of.stage) {
               case _fill_stage::currently_filling:
                  //
                  // When we're filling (i.e. before we're sorting), we're just scanning through 
                  // the `shared_datastore` and copying pointers to any items that match our filter 
                  // parameters. All we have to do, then, is see if the newly-inserted item would 
                  // fall within the range we've copied, and see if it matches our chosen params. 
                  // If so, we just pop it right in!
                  //
                  if (first >= this->_ongoing_fill.progress)
                     return;
                  if (last + 1 > this->_ongoing_fill.progress)
                     last = this->_ongoing_fill.progress - 1;
                  [[fallthrough]];
               case _fill_stage::currently_sorting:
                  //
                  // Sorting, meanwhile, just pulls from the unsorted list.
                  //
                  for (size_t i = first; i <= last; ++i) {
                     auto* entry = source.item_at_row(i);
                     if (!entry)
                        continue;
                     if (!this->_entry_matches_params(*entry, params))
                        continue;
                     of.unsorted.push_back(entry);
                  }
                  return;
            }

            //
            // If we made it here, then we're not currently fetching. We'll want to make our 
            // changes directly to our model state.
            //

            for (size_t i = first; i <= last; ++i) {
               auto* entry = source.item_at_row(i);
               if (!this->_entry_matches_params(*entry, params))
                  continue;

               auto it  = this->_insertion_point_for(*entry);
               int  pos = it - sorted.begin();
               this->beginInsertRows({}, pos, pos);
               sorted.insert(it, entry);
               this->endInsertRows();
            }
         });
         QObject::connect(&source, &shared_datastore::rowsAboutToBeRemoved, this, [this](size_t first, size_t last) {
            auto& sorted = this->_forms;
            auto& unsorted = this->_ongoing_fill.unsorted;
            if (sorted.empty() && unsorted.empty()) // this check, in conjunction with the allDataCleared signal, means we don't need to do any advanced processing for unloading all data
               return;
            auto& source = shared_datastore::get();
            //
            // During a fill operation, before sorting, U many entries from the to-be-removed range will be 
            // present across all relevant lists. The first V of those entries will be in the sorted list, 
            // and the last W of those entries will be in the unsorted list:
            // 
            //    To-Be-Removed:    0 1 2 3 4 5 | 6 7
            //    Sorted:           2 3 5
            //    Unsorted:         6 7
            // 
            // Note that if we ever change sorting to pull from the rear of the unsorted list, then we'll 
            // have to flip this on its head, with the unsorted list being the first half and the sorted 
            // list being the last half.
            //
            auto&  of = this->_ongoing_fill;
            size_t i  = first;
            if (of.stage == _fill_stage::currently_filling) {
               if (first >= of.progress)
                  //
                  // We haven't even reached the to-be-removed elements yet. Bail out.
                  //
                  return;

               size_t cap = last;
               if (cap > of.progress)
                  cap = of.progress;

               size_t continue_from = 0;
               bool   any_removals = false;
               for (; i < cap; ++i) {
                  auto* entry = source.item_at_row(i);
                  assert(entry != nullptr);
                  for (size_t j = continue_from; j < unsorted.size(); ++j) {
                     if (unsorted[j] == entry) {
                        continue_from = j + 1;
                        any_removals  = true;
                        unsorted[j]   = nullptr;
                        break;
                     }
                  }
               }
               if (any_removals) {
                  std::erase(unsorted, nullptr);
               }
               //
               // And fall through to pruning the sorted list.
               //
            }
            //
            // Prune the sorted list.
            //
            bool should_emit_signals = of.stage != _fill_stage::currently_filling && of.stage != _fill_stage::currently_sorting;
            for (; i <= last; ++i) {
               auto* entry = source.item_at_row(i);
               assert(entry != nullptr);
               auto it = std::find(sorted.begin(), sorted.end(), entry);
               if (it == sorted.end())
                  continue;
               if (should_emit_signals) {
                  auto index = std::distance(sorted.begin(), it);
                  this->beginRemoveRows({}, index, index);
               }
               sorted.erase(it);
               if (should_emit_signals) {
                  this->endRemoveRows();
               }
            }
         });
         QObject::connect(&source, &shared_datastore::allDataCleared, this, [this]() {
            this->_forms.clear();
            this->_ongoing_fill.unsorted.clear();
            this->_ongoing_fill.stage    = _fill_stage::inactive;
            this->_ongoing_fill.progress = 0;
            this->_ongoing_fill.timer.stop();
            if (do_fill_diagnostics) {
               this->_fill_diagnostics.ticks_overlap = true;
            }
         });
         QObject::connect(&source, &shared_datastore::editorIDChanged, this, [this](const item_type& item, const QString& prior) {
            bool should_emit_signals = true;
            switch (this->_ongoing_fill.stage) {
               case _fill_stage::currently_filling:
               case _fill_stage::currently_sorting:
                  should_emit_signals = false;
                  break;
            }
            this->_re_sort_item(item, prior, should_emit_signals);
         });
         QObject::connect(&source, &shared_datastore::itemExclusionStateChanged, this, [this](const item_type& entry, bool exclude_now) {
            this->_on_item_exclusion_state_changed(entry, exclude_now);
         });

         auto& fic = dovahkit::subsystems::form_info_cache::core::get();
         QObject::connect(&fic, &std::decay_t<decltype(fic)>::cachedTopicChanged, this, [this](dovah::form_stub& stub) {
            this->_force_recheck_filter(stub);
         });
      }

      bool Model::_is_showing_none_item() const {
         return this->_last_completed_fill_params.allow_none;
      }
      size_t Model::_map_form_index_to_row(size_t i) const {
         return (_is_showing_none_item() ? 1 : 0) + i + this->_subtypes.size();
      }
      size_t Model::_map_row_to_form_index(size_t r) const {
         if (_is_showing_none_item()) {
            if (r == 0)
               return (size_t)-1;
            --r;
         }
         const auto subtype_count = this->_subtypes.size();
         if (r >= subtype_count)
            return r - subtype_count;
         return (size_t)-1;
      }
      const Model::item_type* Model::_map_row_to_item(size_t r) const {
         if (this->_forms.empty())
            return nullptr;
         if (_is_showing_none_item()) {
            if (r == 0)
               return nullptr;
            --r;
         }
         const auto subtype_count = this->_subtypes.size();
         if (r < subtype_count)
            return nullptr;
         r -= subtype_count;
         if (r >= this->_forms.size())
            return nullptr;
         return this->_forms[r];
      }
      const Model::cached_subtype* Model::_map_row_to_subtype(size_t r) const {
         if (_is_showing_none_item()) {
            if (r == 0)
               return nullptr;
            --r;
         }
         if (r >= this->_subtypes.size())
            return nullptr;
         return &this->_subtypes[r];
      }
      bool Model::_row_is_none(size_t r) const {
         return _is_showing_none_item() && r == 0;
      }

      #pragma region QAbstractItemModel boilerplate
         #pragma region Fetching
            /*virtual*/ void Model::fetchMore(const QModelIndex& parent) /*override*/ {
               this->_nextFillStep();
            }
            /*virtual*/ bool Model::canFetchMore(const QModelIndex& parent) const /*override*/ {
               switch (this->_ongoing_fill.stage) {
                  case _fill_stage::currently_filling:
                  case _fill_stage::currently_sorting:
                     return true;
               }
               return false;
            }
         #pragma endregion
         #pragma region Hierarchy
            /*virtual*/ QModelIndex Model::index(int row, int column, const QModelIndex& parent) const /*override*/ {
               if (row < 0)
                  return {};
               if (!this->hasIndex(row, column, parent))
                  return {};
               if (_row_is_none(row) || _map_row_to_subtype(row)) {
                  return this->createIndex(row, column, nullptr);
               } else {
                  auto   size       = this->_forms.size();
                  size_t form_index = _map_row_to_form_index(row);
                  if (form_index >= size)
                     return {};
                  return this->createIndex(row, column, const_cast<item_type*>(this->_forms[form_index]));
               }
            }
            /*virtual*/ QModelIndex Model::parent(const QModelIndex& index) const /*override*/ {
               return {};
            }
            /*virtual*/ int Model::rowCount(const QModelIndex& parent) const /*override*/ {
               switch (this->_ongoing_fill.stage) {
                  case _fill_stage::currently_filling:
                  case _fill_stage::currently_sorting:
                     return 0;
               }
               size_t row = this->_forms.size() + this->_subtypes.size();
               if (_is_showing_none_item())
                  ++row;
               return row;
            }
            /*virtual*/ int Model::columnCount(const QModelIndex& item) const /*override*/ {
               return 1;
            }
         #pragma endregion
         #pragma region Data
            /*virtual*/ QVariant Model::data(const QModelIndex& index, int role) const /*override*/ {
               switch (this->_ongoing_fill.stage) {
                  case _fill_stage::currently_filling:
                  case _fill_stage::currently_sorting:
                     return {};
               }
               if (!index.isValid())
                  return {};
               int i = index.row();

               if (i == 0 && this->_last_completed_fill_params.allow_none) {
                  switch (role) {
                     case FormStubRole:
                        return QVariant::fromValue((dovah::form_stub*)nullptr);
                     case SubtypeRole:
                        return (uint32_t)0;
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        if (!this->_override_text_for_none.isEmpty())
                           return this->_override_text_for_none;
                        return tr("NONE");
                  }
               }
               const auto* form_info = _map_row_to_item(i);
               const auto* subtype   = _map_row_to_subtype(i);
               if (form_info) {
                  switch (role) {
                     case FormStubRole:
                        return QVariant::fromValue(form_info->stub);
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        assert(form_info->stub != nullptr);
                        if (form_info->editorID.isEmpty()) {
                           return tr("<Unnamed: %1>").arg(QString("%1").arg(form_info->stub->formID, 8, 16, QChar('0')).toUpper());
                        }
                        return form_info->editorID;
                  }
                  return {};
               }
               if (subtype) {
                  switch (role) {
                     case SubtypeRole:
                        return subtype->signature;
                     case Qt::DisplayRole:
                     case Qt::ToolTipRole:
                        return subtype->name;
                  }
                  return {};
               }
               return {};
            }
            /*virtual*/ Qt::ItemFlags Model::flags(const QModelIndex& index) const /*override*/ {
               if (!index.isValid())
                  return Qt::NoItemFlags;
               return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
            }
         #pragma endregion
      #pragma endregion
               
      #pragma region DKCustomFormFilterableModelMixin overrides
         /*virtual*/ void Model::recheck_custom_filter_for_all_forms() /*override*/ {
            this->forceRefill();
         }
         /*virtual*/ void Model::recheck_custom_filter_for_form(dovah::form_stub& stub) /*override*/ {
            this->forceRecheckFilterOn(stub);
         }
      #pragma endregion

      bool Model::_entry_matches_params(const item_type& item, const filter_parameters& params) const {
         if (item.type != dovah::form_type::topic)
            return false;
         if (item.stub) {
            if (this->_custom_filter) {
               if (!this->_custom_filter->form_matches(*item.stub))
                  return false;
            }
            auto& fic = dovahkit::subsystems::form_info_cache::core::get();
            if (auto* info = fic.get_topic_info(*item.stub)) {
               if (info->subtype_signature != 'CUST')
                  return false;
            }
         }
         return true;
      }

      decltype(Model::_forms)::iterator Model::_insertion_point_for(const item_type& item) {
         if (!item.stub)
            //
            // The "NONE" entry should always be prepended to the top of the sorted list.
            //
            return this->_forms.begin();

         return std::upper_bound(
            this->_forms.begin(),
            this->_forms.end(),
            &item,
            [](const item_type* a, const item_type* b) {
               if (!a->stub && b->stub)
                  return true;
               if (!b->stub && a->stub)
                  return false;
               return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
            }
         );
      }

      void Model::_refill(const filter_parameters& params) {
         auto& of = this->_ongoing_fill;
         of.stage = _fill_stage::currently_filling;
         {  // Clear OF state, in case we're interrupting an ongoing fill operation with new parameters.
            of.unsorted.clear();
            of.progress = 0;
         }
         this->_forms.clear();
         of.params = params;
         if constexpr (do_fill_diagnostics) {
            this->_fill_diagnostics = {};
         }
         
         auto&  editor = DovahKitCore::get();
         size_t total  = editor.count_forms_of_type(dovah::form_type::topic);
         if (total < estimated_items_possible_in_one_tick) {
            //
            // We *think* we can get this task done in just one tick, so let's not even bother 
            // deferring to the timer.
            //
            QModelIndex dummy;
            this->fetchMore(dummy);
            if (of.stage == _fill_stage::concluding) { // And we were right!
               of.timer.stop(); // stop the timer, in case we're interrupting a previous fill operation
               return;
            }
         }
         of.timer.start();
      }
      void Model::_nextFillStep() {
         auto& of = this->_ongoing_fill;
         switch (of.stage) {
            case _fill_stage::inactive:
            case _fill_stage::concluding:
               return;
         }
         of.ticker.restart();
         if (of.stage == _fill_stage::currently_filling) {
            if (this->_fillGrabMore())
               of.stage = _fill_stage::currently_sorting;

            if (of.ticker.elapsed() >= max_fill_tick_duration)
               return;
            if (do_fill_diagnostics) {
               this->_fill_diagnostics.ticks_overlap = true;
            }
         }
         if (of.stage == _fill_stage::currently_sorting) {
            if (this->_fillSortMore()) {
               this->_finalizeFill();
            }
         }
      }
      bool Model::_fillGrabMore() {
         auto& of = this->_ongoing_fill;
         assert(of.stage == _fill_stage::currently_filling);
         if (do_fill_diagnostics) {
            ++this->_fill_diagnostics.ticks_to_grab;
         }

         auto& unsorted = of.unsorted;
         auto& source   = shared_datastore::get();
         auto  start    = of.progress;
         auto  max      = source.size();
         unsorted.reserve(unsorted.size() + 500);

         for (int i = start; i < max; ++i) {
            if (of.ticker.elapsed() > max_fill_tick_duration)
               break;
            ++of.progress;
            auto* entry = source.item_at_row(i);
            if (!entry)
               continue;
            auto* stub = entry->stub;
            if (!stub) {
               //
               // This model doesn't treat "NONE" as a form. Simpler to concatenate the 
               // subtype and form lists that way.
               //
               continue;
            }
            if (!this->_entry_matches_params(*entry, of.params))
               continue;
            if (entry->default_exclude_from_listings && !this->willNeverDefaultExcludeForm(*stub))
               continue;
            unsorted.push_back(entry);
         }
         return (of.progress == max);
      }
      bool Model::_fillSortMore() {
         auto& of = this->_ongoing_fill;
         assert(of.stage == _fill_stage::currently_sorting);
         if (do_fill_diagnostics) {
            ++this->_fill_diagnostics.ticks_to_sort;
         }
         auto&  unsorted = of.unsorted;
         auto&  sorted   = this->_forms;
         size_t cap      = unsorted.size();
         if (sorted.empty())
            sorted.reserve(cap);
         size_t sorted_this_time = 0;
         for (int i = 0; i < cap; ++i) {
            if (i > 0 && of.ticker.elapsed() > max_fill_tick_duration)
               break;
            ++sorted_this_time;
            auto* entry = unsorted[i];
            sorted.insert(this->_insertion_point_for(*entry), entry);
         }
         unsorted.erase(unsorted.begin(), unsorted.begin() + sorted_this_time); // remove the elements we've just sorted
         return unsorted.empty();
      }
      void Model::_finalizeFill() {
         auto& of = this->_ongoing_fill;
         of.timer.stop();
         of.stage    = _fill_stage::concluding;
         of.progress = 0;
         if (!this->_forms.empty()) {
            this->beginResetModel();
            this->endResetModel();
         }
         this->_last_completed_fill_params = of.params;
         emit beforeFilled();
         emit filled();
         QTimer::singleShot(
            0,
            [this]() {
               if (this->_ongoing_fill.stage == _fill_stage::concluding)
                  this->_ongoing_fill.stage = _fill_stage::inactive;
            }
         );
      }

      void Model::_re_sort_item(const item_type& item, std::optional<QString> prior_name, bool emit_model_sync_signals) {
         auto& list = this->_forms;
            
         auto entry_it = std::find(list.begin(), list.end(), &item);
         if (entry_it == list.end())
            return;
         size_t from = std::distance(list.begin(), entry_it);
         size_t to;
         bool   moving_upward_in_list;
         {
            auto dst_it = this->_insertion_point_for(item);
            //
            // Can't use the iterator directly because we'll be doing a removal first, which will 
            // invalidate it.
            //
            to = std::distance(list.begin(), dst_it);
            moving_upward_in_list = dst_it < entry_it;
         }
         if (emit_model_sync_signals) {
            size_t from_row = _map_form_index_to_row(from);
            size_t to_row   = _map_form_index_to_row(to);
            bool allowed = this->beginMoveRows(
               {},
               from_row, // first to move
               from_row, // last  to move
               {},
               to_row
            );
            if (!allowed) // since we're only moving a single row, the only condition that could cause this is a no-op move (i.e. from N to N)
               return;
         }
         if (!moving_upward_in_list) {
            //
            // We move `entry` by first removing it from the list, and then inserting it into the 
            // list at the desired index. If we're moving `entry` downward within the list, then 
            // its removal will displace the intended destination by -1.
            //
            --to;
         }
         list.erase(entry_it);
         list.insert(list.begin() + to, &item);
         if (emit_model_sync_signals) {
            this->endMoveRows();
         }
      }

      void Model::_on_item_exclusion_state_changed(const item_type& entry, bool exclude_now) {
         auto& of = this->_ongoing_fill;

         const bool  is_fetching_data = (of.stage == _fill_stage::currently_filling || of.stage == _fill_stage::currently_sorting);
         const auto& params           = is_fetching_data ? of.params : this->_last_completed_fill_params;
         if (!this->_entry_matches_params(entry, params))
            return;

         if (exclude_now && entry.stub) {
            exclude_now = !this->willNeverDefaultExcludeForm(*entry.stub);
         }

         if (is_fetching_data) {
            auto it = std::find(of.unsorted.begin(), of.unsorted.end(), &entry);
            if (it != of.unsorted.end()) {
               if (exclude_now)
                  of.unsorted.erase(it);
               return;
            }
            //
            // No branch for if the desired form isn't present in the unsorted list. It's possible 
            // that we just haven't grabbed it yet, so we don't want to add it to the list here and 
            // potentially end up with duplicates in that list.
            //

            if (of.stage != _fill_stage::currently_sorting)
               return;

            //
            // If we're sorting, then we need to remove to-be-excluded forms from the sorted list. 
            // Moreover, we're done grabbing forms, so if our form isn't in either list, then we 
            // need to add it to the unsorted list (where it'll eventually be pulled into the right 
            // spot in the sorted list).
            //

            it = std::find(this->_forms.begin(), this->_forms.end(), &entry);
            if (it == this->_forms.end()) {
               if (!exclude_now)
                  of.unsorted.push_back(&entry);
            } else {
               if (exclude_now)
                  this->_forms.erase(it);
            }
            return;
         }

         if (exclude_now) {
            this->_force_remove_item(entry);
         } else {
            this->_force_insert_item(entry);
         }
      }

      void Model::_force_recheck_filter(const dovah::form_stub& stub) {
         const auto* item = shared_datastore::get().item_for_stub(&stub);
         if (!item)
            return;

         auto& of = this->_ongoing_fill;

         const bool  is_fetching_data = (of.stage == _fill_stage::currently_filling || of.stage == _fill_stage::currently_sorting);
         const auto& params = is_fetching_data ? of.params : this->_last_completed_fill_params;

         bool show = this->_entry_matches_params(*item, params);
         if (show)
            if (item->default_exclude_from_listings && !this->willNeverDefaultExcludeForm(*item->stub))
               show = false;
         
         if (is_fetching_data) {
            auto it = std::find(of.unsorted.begin(), of.unsorted.end(), item);
            if (it != of.unsorted.end()) {
               if (!show)
                  of.unsorted.erase(it);
               return;
            }
            //
            // No branch for if the desired form isn't present in the unsorted list. It's possible 
            // that we just haven't grabbed it yet, so we don't want to add it to the list here and 
            // potentially end up with duplicates in that list.
            //

            if (of.stage != _fill_stage::currently_sorting)
               return;

            //
            // If we're sorting, then we need to remove to-be-excluded forms from the sorted list. 
            // Moreover, we're done grabbing forms, so if our form isn't in either list, then we 
            // need to add it to the unsorted list (where it'll eventually be pulled into the right 
            // spot in the sorted list).
            //

            it = std::find(this->_forms.begin(), this->_forms.end(), item);
            if (it == this->_forms.end()) {
               if (show)
                  of.unsorted.push_back(item);
            } else {
               if (!show)
                  this->_forms.erase(it);
            }
            return;
         }

         if (!show) {
            this->_force_remove_item(*item);
         } else {
            this->_force_insert_item(*item);
         }
      }

      void Model::_force_insert_item(const item_type& item, bool emit_model_sync_signals) {
         if (this->_contains_item(item))
            return;
            
         auto dst_it = this->_insertion_point_for(item);
         if (emit_model_sync_signals) {
            auto index = std::distance(this->_forms.begin(), dst_it);
            auto row   = _map_form_index_to_row(index);
            this->beginInsertRows({}, row, row);
         }
         this->_forms.insert(dst_it, &item);
         if (emit_model_sync_signals) {
            this->endInsertRows();
         }
      }
      void Model::_force_remove_item(const item_type& item, bool emit_model_sync_signals) {
         auto it = std::find(this->_forms.begin(), this->_forms.end(), &item);
         if (it == this->_forms.end())
            return;

         if (emit_model_sync_signals) {
            auto index = std::distance(this->_forms.begin(), it);
            auto row   = _map_form_index_to_row(index);
            this->beginRemoveRows({}, row, row);
         }
         this->_forms.erase(it);
         if (emit_model_sync_signals) {
            this->endRemoveRows();
         }
      }

      void Model::updateParameters(const filter_parameters& params) {
         this->_refill(params);
      }

      int Model::rowOf(dovah::form_stub* stub) {
         if (!stub) {
            if (this->_is_showing_none_item())
               return 0;
            return -1;
         }
         for (size_t i = 0; i < this->_forms.size(); ++i) {
            auto& item = this->_forms[i];
            if (item->stub == stub)
               return _map_form_index_to_row(i);
         }
         return 01;
      }
      int Model::rowOf(uint32_t subtype_signature) {
         if (subtype_signature == 0) {
            if (this->_is_showing_none_item())
               return 0;
            return -1;
         }
         auto& list = dovah::dialogue::all_topic_subtypes;
         for (size_t i = 0; i < list.size(); ++i) {
            if (list[i].signature == subtype_signature) {
               size_t row = i;
               if (this->_is_showing_none_item())
                  ++row;
               return row;
            }
         }
         return -1;
      }

      QString Model::overrideTextForNone() const {
         return this->_override_text_for_none;
      }
      void Model::setOverrideTextForNone(QString s) {
         if (s == this->_override_text_for_none)
            return;
         this->_override_text_for_none = s;

         if (this->_ongoing_fill.stage != _fill_stage::inactive)
            return;
         {
            auto& lcfp = this->_last_completed_fill_params;
            if (!lcfp.allow_none)
               //
               // There's no "NONE" option to update.
               //
               return;
         }
         auto qmi = this->index(0, 0, {});
         emit dataChanged(qmi, qmi, { Qt::DisplayRole, Qt::ToolTipRole });
      }

      bool Model::willNeverDefaultExcludeForm(dovah::form_stub& desired) const {
         for (auto* stub : this->_force_included_forms)
            if (stub == &desired)
               return true;
         return false;
      }
      void Model::setFormNeverDefaultExcluded(dovah::form_stub& stub, bool force_include) {
         auto& fif    = this->_force_included_forms;
         auto  fif_it = std::find(
            fif.begin(),
            fif.end(),
            &stub
         );

         if (force_include) {
            if (fif_it == fif.end()) {
               fif.push_back(&stub);
            } else {
               return;
            }
         } else {
            if (fif_it != fif.end()) {
               fif.erase(fif_it);
            } else {
               return;
            }
         }

         //
         // Now update the model state:
         //

         auto* item = shared_datastore::get().item_for_stub(&stub);
         if (!item || !item->default_exclude_from_listings)
            return;
         this->_on_item_exclusion_state_changed(*item, !force_include);
      }

      void Model::forceRefill() {
         this->_refill(this->isFilling() ? this->_ongoing_fill.params : this->_last_completed_fill_params);
      }
      void Model::forceRecheckFilterOn(const dovah::form_stub& stub) {
         this->_force_recheck_filter(stub);
      }
   #pragma endregion
}