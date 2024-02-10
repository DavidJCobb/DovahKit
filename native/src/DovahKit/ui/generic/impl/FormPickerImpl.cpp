#include "FormPickerImpl.h"
#include "../../../editor/core.h"
#include "../../../editor/form_stub_meta_type.h"
#include <QElapsedTimer>

/*

   ADVANTAGES

    - No lag when populating the form dropdown, as we populate and sort it across multiple ticks.
      This means that even statics, which contain thousands of forms, can be listed quickly.
 
*/

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

namespace FormPickerImpl {
   #pragma region FormPickerSharedUnderlyingModel
   FormPickerSharedUnderlyingModel::FormPickerSharedUnderlyingModel() : QAbstractItemModel(&DovahKitCore::get()) {
      this->forms.append(new item); // none
      //
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &FormPickerSharedUnderlyingModel::rebuild);
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         emit this->allDataCleared();
         //
         this->beginResetModel();
         this->forms.clear();
         //
         this->forms.append(new item); // none
         //
         this->endResetModel();
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
         auto size = this->forms.size();
         for (int i = 0; i < size; ++i) {
            auto& n = this->forms[i];
            if (n->stub == stub) {
               this->beginRemoveRows(QModelIndex(), i, i);
               this->forms.remove(i);
               delete n;
               this->endRemoveRows();
               return;
            }
         }
      });
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         auto i = this->forms.size();
         this->beginInsertRows(QModelIndex(), i, i);
         auto n = new item;
         n->stub     = stub;
         n->type     = stub->formType;
         n->editorID = stub->get_editor_id();
         this->forms.append(n);
         this->endInsertRows();
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         auto size = this->forms.size();
         for (int i = 0; i < size; ++i) {
            auto* n = this->forms[i];
            if (n->stub == stub) {
               auto prior = n->editorID;
               n->editorID = stub->get_editor_id();
               if (prior != n->editorID) {
                  emit editorIDChanged(n, prior);
               }
               auto index = this->index(i, 0, QModelIndex());
               emit dataChanged(index, index, { Qt::DisplayRole, Qt::ToolTipRole }); // in case the editor ID changed
               return;
            }
         }
      });
      QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
         auto size = this->forms.size();
         for (int i = 0; i < size; ++i) {
            auto* n = this->forms[i];
            if (n->stub == stub) {
               auto index = this->index(i, 0, QModelIndex());
               emit dataChanged(index, index, { FormIDRole });
               return;
            }
         }
      });
      //
      this->rebuild(); // since this model can be instantiated after data has been loaded
   }

   void FormPickerSharedUnderlyingModel::rebuild() {
      this->beginResetModel();
      //
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
      this->forms.reserve(count);
      //
      for (const auto& info : dovah::form_types) {
         if (_should_exclude_form_type(info.form_type))
            continue;
         editor.for_each_form_of_type(info.form_type, [this](dovah::form_stub* stub) {
            if (_should_exclude_form(stub))
               return false;
            auto i = new item;
            i->stub     = stub;
            i->type     = stub->formType;
            i->editorID = stub->get_editor_id();
            this->forms.append(i);
            return false;
         });
      }
      this->endResetModel();
   }

   QModelIndex FormPickerSharedUnderlyingModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->hasIndex(row, column, parent))
         return QModelIndex();
      auto size = this->forms.size();
      if (row < 0 || row >= size)
         return QModelIndex();
      return this->createIndex(row, column, this->forms[row]);
   }
   QModelIndex FormPickerSharedUnderlyingModel::parent(const QModelIndex& index) const {
      return QModelIndex();
   }
   int FormPickerSharedUnderlyingModel::rowCount(const QModelIndex& parent) const {
      return this->forms.size();
   }
   int FormPickerSharedUnderlyingModel::columnCount(const QModelIndex& item) const {
      return 1;
   }
   Qt::ItemFlags FormPickerSharedUnderlyingModel::flags(const QModelIndex& index) const {
      if (!index.isValid())
         return Qt::NoItemFlags;
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   }
   QVariant FormPickerSharedUnderlyingModel::data(const QModelIndex& index, int role) const {
      if (!index.isValid())
         return QVariant();
      auto* base = (item*) index.internalPointer();
      if (!base)
         return QVariant();
      auto* stub = base->stub;
      switch (role) {
         case FormIDRole:
            if (!stub)
               return 0;
            return stub->formID;
         case FormStubRole:
            return QVariant::fromValue(stub);
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            if (!stub)
               return tr("NONE");
            return base->editorID;
      }
      return QVariant();
   }

   const FormPickerSharedUnderlyingModel::item* FormPickerSharedUnderlyingModel::itemAtRow(int i) const noexcept {
      if (i < 0 || i >= this->forms.size())
         return nullptr;
      return this->forms[i];
   }
   #pragma endregion

   #pragma region FormPickerIterativeProxy
   FormPickerIterativeModel::FormPickerIterativeModel(QObject* parent) : QAbstractItemModel(parent) {
      QObject::connect(&this->ongoing_fill.timer, &QTimer::timeout, this, [this]() {
         this->fetchMore(QModelIndex());
      });
      //
      auto& source = FormPickerSharedUnderlyingModel::get();
      QObject::connect(&source, &QAbstractItemModel::rowsInserted, this, [this](const QModelIndex& parent, int first, int last) {
         auto& source   = FormPickerSharedUnderlyingModel::get();
         auto& sorted   = this->stubs;
         auto& unsorted = this->ongoing_fill.unsorted;
         //
         int start = first;
         int end   = last + 1;
         if (this->ongoing_fill.filling && !this->ongoing_fill.sorting) {
            if (first >= this->ongoing_fill.progress)
               return;
            if (end > this->ongoing_fill.progress)
               end = this->ongoing_fill.progress;
            for (int i = first; i < end; ++i) {
               auto* entry = source.itemAtRow(i);
               if (!entry)
                  continue;
               unsorted.push_back(entry);
            }
            return;
         }
         QModelIndex dummy;
         for (int i = start; i < end; ++i) {
            auto* entry = source.itemAtRow(i);
            auto it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
            });
            int pos = it - sorted.begin();
            if (!this->ongoing_fill.filling)
               this->beginInsertRows(dummy, pos, pos); // handles persistent model indexes for us
            sorted.insert(it, entry);
            if (!this->ongoing_fill.filling)
               this->endInsertRows(); // handles persistent model indexes for us
         }
      });
      QObject::connect(&source, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
         auto& sorted   = this->stubs;
         auto& unsorted = this->ongoing_fill.unsorted;
         if (sorted.empty() && unsorted.empty()) // this check, in conjunction with the allDataCleared signal, means we don't need to do any advanced processing for unloading all data
            return;
         auto& source   = FormPickerSharedUnderlyingModel::get();
         //
         int cap = last;
         if (this->ongoing_fill.filling && !this->ongoing_fill.sorting) {
            if (first >= this->ongoing_fill.progress)
               return;
            cap = last + 1;
            if (cap > this->ongoing_fill.progress)
               cap = this->ongoing_fill.progress;
            for (int i = first; i < cap; ++i)
               unsorted.remove(first);
            return;
         }
         for (int i = first; i <= cap; ++i) {
            auto* entry = source.itemAtRow(i);
            if (entry) {
               int index = sorted.indexOf(entry);
               if (index >= 0) {
                  this->beginRemoveRows(parent, index, index); // handles persistent model indexes for us
                  sorted.remove(index);
                  this->endRemoveRows();
               }
            }
         }
      });
      QObject::connect(&source, &FormPickerSharedUnderlyingModel::allDataCleared, this, [this]() {
         this->stubs.clear();
         this->ongoing_fill.unsorted.clear();
         this->ongoing_fill.filling   = false;
         this->ongoing_fill.sorting   = false;
         this->ongoing_fill.post_fill = false;
         this->ongoing_fill.progress  = 0;
         this->ongoing_fill.timer.stop();
         this->_resetFillDiagnostics();
      });
      QObject::connect(&source, &FormPickerSharedUnderlyingModel::editorIDChanged, this, [this](const item* entry, const QString& prior) {
         auto& source = FormPickerSharedUnderlyingModel::get();
         auto& sorted = this->stubs;
         //
         if (this->ongoing_fill.filling && !this->ongoing_fill.sorting)
            return;
         int i = sorted.indexOf(entry);
         if (i < 0)
            return;
         iterator    it;
         QModelIndex parent;
         if (entry->editorID.compare(prior, Qt::CaseInsensitive) < 0) { // moving it to a spot higher in the list
            it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
            });
            int to = it - sorted.begin();
            this->beginMoveRows(parent, i, i, parent, to); // when moving items anywhere except down in the same parent, the last arg is the destination index
            sorted.remove(i);
            sorted.insert(to, entry);
            this->endMoveRows(); // handles persistent model indexes for us
         } else { // moving it to a spot later in the list
            it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
            });
            int to = it - sorted.begin() - 1;
            this->beginMoveRows(parent, i, i, parent, to + 1); // when moving items down in the same parent, the last arg is the spot AFTER the destination index, because this API is cursed
            sorted.remove(i);
            sorted.insert(to, entry);
            this->endMoveRows(); // handles persistent model indexes for us
         }
      });
   }

   void FormPickerIterativeModel::_resetFillDiagnostics() {
      if constexpr (do_fill_diagnostics) {
         this->fill_diagnostics.ticks_to_grab = 0;
         this->fill_diagnostics.ticks_to_sort = 0;
      }
   }
   bool FormPickerIterativeModel::_fillGrabMore() {
      if (!this->ongoing_fill.filling)
         return false;
      if (do_fill_diagnostics) {
         ++this->fill_diagnostics.ticks_to_grab;
      }
      auto& unsorted = this->ongoing_fill.unsorted;
      auto& source   = FormPickerSharedUnderlyingModel::get();
      auto  start    = this->ongoing_fill.progress;
      auto  max      = source.rowCount(QModelIndex());
      unsorted.reserve(unsorted.size() + 500);
      for (int i = start; i < max; ++i) {
         if (this->ongoing_fill.ticker.elapsed() > max_fill_tick_duration)
            break;
         ++this->ongoing_fill.progress;
         auto* entry = source.itemAtRow(i);
         if (!entry)
            continue;
         auto* stub = entry->stub;
         if (stub) {
            if (!this->ongoing_fill.form_types.isEmpty() && !this->ongoing_fill.form_types.contains(entry->type))
               continue;
         } else {
            if (!this->ongoing_fill.allow_none)
               continue;
         }
         unsorted.push_back(entry);
      }
      return (this->ongoing_fill.progress == max);
   }
   bool FormPickerIterativeModel::_fillSortMore() {
      if (!this->ongoing_fill.filling)
         return false;
      if (do_fill_diagnostics) {
         ++this->fill_diagnostics.ticks_to_sort;
      }
      auto& unsorted = this->ongoing_fill.unsorted;
      auto& sorted   = this->stubs;
      int   cap      = unsorted.size();
      if (sorted.empty())
         sorted.reserve(cap);
      int sorted_this_time = 0;
      for (int i = 0; i < cap; ++i) {
         if (this->ongoing_fill.ticker.elapsed() > max_fill_tick_duration)
            break;
         ++sorted_this_time;
         auto* entry = unsorted[i];
         if (!entry->stub) { // "NONE" special-case
            sorted.prepend(entry);
            continue;
         }
         auto it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
            if (!a->stub && b->stub)
               return true;
            if (!b->stub && a->stub)
               return false;
            return a->editorID.compare(b->editorID, Qt::CaseInsensitive) < 0;
         });
         sorted.insert(it, entry);
      }
      //unsorted.remove(0, cap);
      unsorted.remove(0, sorted_this_time);
      return unsorted.empty();
   }

   void FormPickerIterativeModel::fetchMore(const QModelIndex& parent) {
      if (!this->ongoing_fill.filling)
         return;
      this->ongoing_fill.ticker.restart();
      if (!this->ongoing_fill.sorting) {
         if (this->_fillGrabMore())
            this->ongoing_fill.sorting = true;
         if (this->ongoing_fill.ticker.elapsed() >= max_fill_tick_duration)
            return;
         if (do_fill_diagnostics) {
            this->fill_diagnostics.ticks_overlap = true;
         }
      }
      if (this->_fillSortMore()) {
         this->ongoing_fill.timer.stop();
         this->ongoing_fill.filling   = false;
         this->ongoing_fill.post_fill = true;
         this->ongoing_fill.sorting   = false;
         this->ongoing_fill.progress  = 0;
         if (auto size = this->stubs.size()) {
            this->beginResetModel();
            this->endResetModel();
         }
         emit beforeFilled();
         emit filled();
         QTimer::singleShot(0, [this]() { this->ongoing_fill.post_fill = false; });
      }
   }
   bool FormPickerIterativeModel::canFetchMore(const QModelIndex& parent) const {
      return this->ongoing_fill.filling;
   }

   void FormPickerIterativeModel::refill(bool allow_none, const QVector<dovah::form_type_t>& form_types) {
      this->ongoing_fill.filling = true;
      this->stubs.clear();
      this->ongoing_fill.allow_none = allow_none;
      this->ongoing_fill.form_types = form_types;
      this->ongoing_fill.progress   = 0;
      this->ongoing_fill.unsorted.clear();
      this->_resetFillDiagnostics();
      //
      auto&    editor = DovahKitCore::get();
      uint32_t total  = 0;
      for (auto ft : form_types)
         total += editor.count_forms_of_type(ft);
      if (total < estimated_items_possible_in_one_tick) {
         //
         // We *think* we can get this task done in just one tick, so let's not even bother 
         // deferring to the timer.
         //
         QModelIndex dummy;
         this->fetchMore(dummy);
         if (!this->ongoing_fill.filling) { // And we were right!
            this->ongoing_fill.timer.stop(); // stop the timer, in case we're interrupting a previous fill operation
            return;
         }
      }
      //
      this->ongoing_fill.timer.start();
   }
   void FormPickerIterativeModel::updateParameters(bool allow_none, const QVector<dovah::form_type_t>& form_types) {
      this->refill(allow_none, form_types);
   }
   
   QModelIndex FormPickerIterativeModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->hasIndex(row, column, parent))
         return QModelIndex();
      auto size = this->stubs.size();
      if (row < 0 || row >= size)
         return QModelIndex();
      return this->createIndex(row, column, const_cast<item*>(this->stubs[row]));
   }
   QModelIndex FormPickerIterativeModel::parent(const QModelIndex& index) const {
      return QModelIndex();
   }
   int FormPickerIterativeModel::rowCount(const QModelIndex& parent) const {
      if (this->ongoing_fill.filling)
         return 0;
      return this->stubs.size();
   }
   int FormPickerIterativeModel::columnCount(const QModelIndex& item) const {
      return 1;
   }
   Qt::ItemFlags FormPickerIterativeModel::flags(const QModelIndex& index) const {
      if (!index.isValid())
         return Qt::NoItemFlags;
      return Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemIsEnabled;
   }
   QVariant FormPickerIterativeModel::data(const QModelIndex& index, int role) const {
      if (this->ongoing_fill.filling)
         return QVariant();
      if (!index.isValid())
         return QVariant();
      int i = index.row();
      if (i < 0 || i >= this->stubs.size())
         return QVariant();
      auto* entry = this->stubs[i];
      switch (role) {
         case FormPickerSharedUnderlyingModel::FormIDRole:
            if (!entry->stub)
               return 0;
            return entry->stub->formID;
         case FormPickerSharedUnderlyingModel::FormStubRole:
            return QVariant::fromValue(entry->stub);
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            if (!entry->stub)
               return tr("NONE");
            return entry->editorID;
      }
      return QVariant();
   }

   int FormPickerIterativeModel::indexOf(const dovah::form_stub* stub) const noexcept {
      int size = this->stubs.size();
      for (int i = 0; i < size; ++i)
         if (this->stubs[i]->stub == stub)
            return i;
      return -1;
   }
   int FormPickerIterativeModel::indexOfFormID(dovah::bare_form_id_t id) const noexcept {
      int size = this->stubs.size();
      for (int i = 0; i < size; ++i) {
         auto* stub = this->stubs[i]->stub;
         if (id == 0 && !stub)
            return i;
         if (stub && stub->formID == id)
            return i;
      }
      return -1;
   }
   #pragma endregion
}