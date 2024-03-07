#include "./DKFormPickerModel.h"
#include "dovah/form_stub.h"
#include "editor/subsystems/papyrus/core.h"
#include "editor/core.h"
#include "editor/form_stub_meta_type.h"

namespace {
   bool _should_exclude_form_type(dovah::form_type_t ft) {
      if (dovah::form_type_info::form_type_is_reference(ft))
         return true;
      if (dovah::form_type_info::lookup(ft).flags & dovah::form_type_info::flag::no_connections)
         return true;
      return false;
   }
   bool _form_type_has_exclusions(dovah::form_type_t ft) {
      if (ft == dovah::form_type::cell)
         return true;
      return false;
   }
   bool _should_exclude_form(const dovah::form_stub* stub) {
      if (stub->formType == dovah::form_type::cell)
         return stub->is_exterior_cell();
      return false;
   }

   constexpr int max_fill_tick_duration = 33; // milliseconds
   #if _DEBUG
      constexpr bool do_fill_diagnostics = true;
      constexpr int  estimated_items_possible_in_one_tick = 1000; // budget for less-async fills. per control, and some windows have several FormPickers, so keep it low.
   #else
      constexpr bool do_fill_diagnostics = false;
      constexpr int  estimated_items_possible_in_one_tick = 2000; // budget for less-async fills. per control, and some windows have several FormPickers, so keep it low.
   #endif
}

namespace ui::impl::DKFormPicker {
   #pragma region shared_datastore
   shared_datastore::shared_datastore() : QObject(&DovahKitCore::get()) {
      this->_forms.push_back(new item_type); // none
      //
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &shared_datastore::_rebuild);
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         emit this->allDataCleared();
         this->_forms.clear();
         this->_forms.push_back(new item_type); // none
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
         auto it = std::find_if(
            this->_forms.begin(),
            this->_forms.end(),
            [stub](const item_type* item) {
               return item->stub == stub;
            }
         );
         if (it == this->_forms.end())
            return;
         size_t index = std::distance(this->_forms.begin(), it);
         emit this->rowsAboutToBeRemoved(index, index);
         this->_forms.erase(it);
      });
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         size_t i    = this->_forms.size();
         auto   item = std::make_unique<item_type>();
         item->stub     = stub;
         item->type     = stub->formType;
         item->editorID = stub->get_editor_id();
         this->_forms.push_back(item.get());
         item.release();

         emit this->rowsInserted(i, i);
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         auto it = std::find_if(
            this->_forms.begin(),
            this->_forms.end(),
            [stub](const item_type* item) {
               return item->stub == stub;
            }
         );
         if (it == this->_forms.end())
            return;

         auto* item  = *it;
         auto  prior = item->editorID;
         item->editorID = stub->get_editor_id();
         if (prior != item->editorID) {
            emit editorIDChanged(*item, prior);
         }
      });
      //
      this->_rebuild(); // since this model can be instantiated after data has been loaded
   }

   void shared_datastore::_rebuild() {
      auto& editor = DovahKitCore::get();
      //
      size_t count = 0;
      for (const auto& info : dovah::form_types) {
         if (_should_exclude_form_type(info.form_type))
            continue;
         if (_form_type_has_exclusions(info.form_type)) {
            editor.for_each_form_of_type(info.form_type, [&count](dovah::form_stub* stub) {
               if (!_should_exclude_form(stub))
                  ++count;
               return false;
            });
         } else {
            count += editor.count_forms_of_type(info.form_type);
         }
      }
      this->_forms.reserve(count);
      //
      for (const auto& info : dovah::form_types) {
         if (_should_exclude_form_type(info.form_type))
            continue;
         editor.for_each_form_of_type(info.form_type, [this](dovah::form_stub* stub) {
            if (_should_exclude_form(stub))
               return false;
            auto item = std::make_unique<item_type>();
            item->stub     = stub;
            item->type     = stub->formType;
            item->editorID = stub->get_editor_id();
            this->_forms.push_back(item.get());
            item.release();
            return false;
         });
      }
   }

   const shared_datastore::item_type* shared_datastore::item_at_row(int i) const noexcept {
      if (i < 0 || i >= this->_forms.size())
         return nullptr;
      return this->_forms[i];
   }
   #pragma endregion

   #pragma region Model
      Model::Model(QObject* parent) {
         QObject::connect(&this->_ongoing_fill.timer, &QTimer::timeout, this, [this]() {
            this->fetchMore({});
         });
         //
         auto& source = shared_datastore::get();
         QObject::connect(&source, &shared_datastore::rowsInserted, this, [this](size_t first, size_t last) {
            auto& source   = shared_datastore::get();
            auto& sorted   = this->_items;
            auto& unsorted = this->_ongoing_fill.unsorted;

            bool is_during_fill = this->_ongoing_fill.stage == _fill_stage::in_progress;

            int start = first;
            int end   = last + 1;
            if (is_during_fill && !this->_ongoing_fill.sorting) {
               if (first >= this->_ongoing_fill.progress)
                  return;
               if (end > this->_ongoing_fill.progress)
                  end = this->_ongoing_fill.progress;
               for (int i = first; i < end; ++i) {
                  auto* entry = source.item_at_row(i);
                  if (!entry)
                     continue;
                  unsorted.push_back(entry);
               }
               return;
            }

            for (int i = start; i < end; ++i) {
               auto* entry = source.item_at_row(i);
               auto  it    = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item_type* a, const item_type* b) {
                  return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
               });
               if (!is_during_fill) {
                  //
                  // If a fill operation isn't ongoing, then we need to send signals. If it is ongoing, 
                  // then don't bother: the end of the fill operation will send signals for everything 
                  // that's been filled.
                  //
                  int pos = it - sorted.begin();
                  this->beginInsertRows({}, pos, pos); // handles persistent model indexes for us
               }
               sorted.insert(it, entry);
               if (!is_during_fill) {
                  this->endInsertRows(); // handles persistent model indexes for us
               }
            }
         });
         QObject::connect(&source, &shared_datastore::rowsAboutToBeRemoved, this, [this](size_t first, size_t last) {
            auto& sorted   = this->_items;
            auto& unsorted = this->_ongoing_fill.unsorted;
            if (sorted.empty() && unsorted.empty()) // this check, in conjunction with the allDataCleared signal, means we don't need to do any advanced processing for unloading all data
               return;
            auto& source   = shared_datastore::get();
            //
            int cap = last;
            if (this->_ongoing_fill.stage == _fill_stage::in_progress && !this->_ongoing_fill.sorting) {
               if (first >= this->_ongoing_fill.progress)
                  return;
               cap = last + 1;
               if (cap > this->_ongoing_fill.progress)
                  cap = this->_ongoing_fill.progress;
               unsorted.erase(unsorted.begin() + first, unsorted.begin() + cap);
               return;
            }
            for (int i = first; i <= cap; ++i) {
               auto* entry = source.item_at_row(i);
               if (!entry)
                  continue;
               auto it = std::find(sorted.begin(), sorted.end(), entry);
               if (it == sorted.end())
                  continue;
               auto index = std::distance(sorted.begin(), it);
               this->beginRemoveRows({}, index, index); // handles persistent model indexes for us
               sorted.erase(it);
               this->endRemoveRows();
            }
         });
         QObject::connect(&source, &shared_datastore::allDataCleared, this, [this]() {
            this->_items.clear();
            this->_ongoing_fill.unsorted.clear();
            this->_ongoing_fill.stage    = _fill_stage::inactive;
            this->_ongoing_fill.sorting  = false;
            this->_ongoing_fill.progress = 0;
            this->_ongoing_fill.timer.stop();
            if (do_fill_diagnostics) {
               this->_fill_diagnostics.ticks_overlap = true;
            }
         });
         QObject::connect(&source, &shared_datastore::editorIDChanged, this, [this](const item_type& entry, const QString& prior) {
            auto& source = shared_datastore::get();
            auto& sorted = this->_items;
            //
            if (this->_ongoing_fill.stage == _fill_stage::in_progress && !this->_ongoing_fill.sorting)
               return;
            
            auto entry_it = std::find(sorted.begin(), sorted.end(), &entry);
            if (entry_it == sorted.end())
               return;
            size_t from = std::distance(sorted.begin(), entry_it);

            const bool moving_upward_in_list = (entry.editorID.compare(prior, Qt::CaseInsensitive) < 0);

            size_t to;
            {
               auto dst_it = std::upper_bound(
                  sorted.begin(),
                  sorted.end(),
                  &entry,
                  [](const item_type* a, const item_type* b) {
                     return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
                  }
               );
               to = std::distance(sorted.begin(), dst_it);
               //
               // Can't use the iterator directly because we'll be doing a removal first, which will 
               // invalidate it.
            }
            this->beginMoveRows(
               {},
               from, // first to move
               from, // last  to move
               {},
               to
            );
            if (!moving_upward_in_list) {
               //
               // We move `entry` by first removing it from the list, and then inserting it into the 
               // list at the desired index. If we're moving `entry` downward within the list, then 
               // its removal will displace the intended destination by -1.
               //
               --to;
            }
            sorted.erase(entry_it);
            sorted.insert(sorted.begin() + to, &entry);
            this->endMoveRows(); // handles persistent model indexes for us
         });
      }

      #pragma region QAbstractItemModel boilerplate
         #pragma region Fetching
            /*virtual*/ void Model::fetchMore(const QModelIndex& parent) /*override*/ {
               auto& of = this->_ongoing_fill;
               if (of.stage != _fill_stage::in_progress)
                  return;
               of.ticker.restart();
               if (!of.sorting) {
                  if (this->_fillGrabMore())
                     of.sorting = true;
                  if (of.ticker.elapsed() >= max_fill_tick_duration)
                     return;
                  if (do_fill_diagnostics) {
                     this->_fill_diagnostics.ticks_overlap = true;
                  }
               }
               if (this->_fillSortMore()) {
                  of.timer.stop();
                  of.stage     = _fill_stage::concluding;
                  of.sorting   = false;
                  of.progress  = 0;
                  if (auto size = this->_items.size()) {
                     this->beginResetModel();
                     this->endResetModel();
                  }
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
            }
            /*virtual*/ bool Model::canFetchMore(const QModelIndex& parent) const /*override*/ {
               return this->_ongoing_fill.stage == _fill_stage::in_progress;
            }
         #pragma endregion
         #pragma region Hierarchy
            /*virtual*/ QModelIndex Model::index(int row, int column, const QModelIndex& parent) const /*override*/ {
               if (!this->hasIndex(row, column, parent))
                  return {};
               auto size = this->_items.size();
               if (row < 0 || row >= size)
                  return {};
               return this->createIndex(row, column, const_cast<item_type*>(this->_items[row]));
            }
            /*virtual*/ QModelIndex Model::parent(const QModelIndex& index) const /*override*/ {
               return {};
            }
            /*virtual*/ int Model::rowCount(const QModelIndex& parent) const /*override*/ {
               if (this->_ongoing_fill.stage == _fill_stage::in_progress)
                  return 0;
               return this->_items.size();
            }
            /*virtual*/ int Model::columnCount(const QModelIndex& item) const /*override*/ {
               return 1;
            }
         #pragma endregion
         #pragma region Data
            /*virtual*/ QVariant Model::data(const QModelIndex& index, int role) const /*override*/ {
               if (this->_ongoing_fill.stage == _fill_stage::in_progress)
                  return {};
               if (!index.isValid())
                  return {};
               int i = index.row();
               if (i < 0 || i >= this->_items.size())
                  return {};
               const auto* entry = this->_items[i];
               switch (role) {
                  case FormStubRole:
                     return QVariant::fromValue(entry->stub);
                  case Qt::DisplayRole:
                  case Qt::ToolTipRole:
                     if (!entry->stub)
                        return tr("NONE");
                     return entry->editorID;
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

      bool Model::_entry_matches_params(const item_type& entry) const {
         auto& params = this->_ongoing_fill.params;
         if (!params.form_types.isEmpty()) {
            if (!params.form_types.contains(entry.type))
               return false;
         }
         {
            bool check_form_script  = !params.scriptname.empty();
            bool check_alias_script = (entry.type == dovah::form_type::quest && !params.scriptname_on_aliases.empty());
            if (check_form_script || check_alias_script) {
               auto& papyrus = dovahkit::subsystems::papyrus::core::get();
               if (check_form_script) {
                  if (!papyrus.form_has_script_attached(*entry.stub, params.scriptname))
                     return false;
               }
               if (check_alias_script) {
                  if (!papyrus.quest_has_script_attached_to_any_alias(*entry.stub, params.scriptname_on_aliases))
                     return false;
               }
            }
         }
         return true;
      }

      void Model::_refill(const filter_parameters& params) {
         auto& of = this->_ongoing_fill;
         of.stage = _fill_stage::in_progress;
         {  // Clear OF state, in case we're interrupting an ongoing fill operation with new parameters.
            of.unsorted.clear();
            of.progress = 0;
         }
         this->_items.clear();
         of.params = params;
         if constexpr (do_fill_diagnostics) {
            this->_fill_diagnostics = {};
         }
         
         auto&  editor = DovahKitCore::get();
         size_t total  = 0;
         for (auto ft : params.form_types)
            total += editor.count_forms_of_type(ft);
         if (total < estimated_items_possible_in_one_tick) {
            //
            // We *think* we can get this task done in just one tick, so let's not even bother 
            // deferring to the timer.
            //
            QModelIndex dummy;
            this->fetchMore(dummy);
            if (of.stage != _fill_stage::in_progress) { // And we were right!
               of.timer.stop(); // stop the timer, in case we're interrupting a previous fill operation
               return;
            }
         }
         of.timer.start();
      }
      bool Model::_fillGrabMore() {
         auto& of = this->_ongoing_fill;
         if (of.stage != _fill_stage::in_progress)
            return false;
         if (do_fill_diagnostics) {
            ++this->_fill_diagnostics.ticks_to_grab;
         }

         auto& unsorted = of.unsorted;
         auto& source   = shared_datastore::get();
         auto  start    = of.progress;
         auto  max      = source.size();
         unsorted.reserve(unsorted.size() + 500);

         bool allow_none = of.params.allow_none || of.params.always_allow_none();

         for (int i = start; i < max; ++i) {
            if (of.ticker.elapsed() > max_fill_tick_duration)
               break;
            ++of.progress;
            auto* entry = source.item_at_row(i);
            if (!entry)
               continue;
            auto* stub = entry->stub;
            if (stub) {
               if (!this->_entry_matches_params(*entry))
                  continue;
            } else {
               if (allow_none)
                  continue;
            }
            unsorted.push_back(entry);
         }
         return (of.progress == max);
      }
      bool Model::_fillSortMore() {
         auto& of = this->_ongoing_fill;
         if (of.stage != _fill_stage::in_progress)
            return false;
         if (do_fill_diagnostics) {
            ++this->_fill_diagnostics.ticks_to_sort;
         }
         auto&  unsorted = of.unsorted;
         auto&  sorted   = this->_items;
         size_t cap      = unsorted.size();
         if (sorted.empty())
            sorted.reserve(cap);
         size_t sorted_this_time = 0;
         for (int i = 0; i < cap; ++i) {
            if (of.ticker.elapsed() > max_fill_tick_duration)
               break;
            ++sorted_this_time;
            auto* entry = unsorted[i];
            if (!entry->stub) { // "NONE" special-case
               //
               // The "NONE" entry should always be prepended to the top of the sorted list.
               //
               sorted.insert(sorted.begin(), entry);
               continue;
            }
            auto it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item_type* a, const item_type* b) {
               if (!a->stub && b->stub)
                  return true;
               if (!b->stub && a->stub)
                  return false;
               return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
            });
            sorted.insert(it, entry);
         }
         unsorted.erase(unsorted.begin(), unsorted.begin() + sorted_this_time); // remove the elements we've just sorted
         return unsorted.empty();
      }

      void Model::updateParameters(const filter_parameters& params) {
         this->_refill(params);
      }

      int Model::indexOf(const dovah::form_stub* stub) const noexcept {
         int size = this->_items.size();
         for (int i = 0; i < size; ++i)
            if (this->_items[i]->stub == stub)
               return i;
         return -1;
      }
   #pragma endregion
}