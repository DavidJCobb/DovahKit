#include "FormPickerImpl.h"
#include "../../../editor/core.h"
#include "../../../editor/form_stub_meta_type.h"

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

   constexpr int make_per_tick = 1500;
   constexpr int sort_per_tick = 1500;
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
         if (_should_exclude_form_type(info.formType))
            continue;
         if (_form_type_has_exclusions(info.formType)) {
            editor.for_each_form_of_type(info.formType, [&count](dovah::form_stub* stub) {
               if (!_should_exclude_form(stub))
                  ++count;
               return false;
            });
         } else {
            count += editor.count_forms_of_type(info.formType);
         }
      }
      this->forms.reserve(count);
      //
      for (const auto& info : dovah::form_types) {
         if (_should_exclude_form_type(info.formType))
            continue;
         editor.for_each_form_of_type(info.formType, [this](dovah::form_stub* stub) {
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
         for (int i = start; i < end; ++i) {
            auto* entry = source.itemAtRow(i);
            auto it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID < b->editorID;
            });
            sorted.insert(it, entry);
         }
      });
      QObject::connect(&source, &QAbstractItemModel::rowsAboutToBeRemoved, this, [this](const QModelIndex& parent, int first, int last) {
         auto& sorted   = this->stubs;
         auto& unsorted = this->ongoing_fill.unsorted;
         if (sorted.empty() && unsorted.empty())
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
                  this->beginRemoveRows(parent, index, index);
                  sorted.remove(index);
                  this->endRemoveRows();
               }
            }
         }
      });
      QObject::connect(&source, &FormPickerSharedUnderlyingModel::allDataCleared, this, [this]() {
         this->stubs.clear();
         this->ongoing_fill.unsorted.clear();
         this->ongoing_fill.filling  = false;
         this->ongoing_fill.sorting  = false;
         this->ongoing_fill.progress = 0;
         this->ongoing_fill.timer.stop();
      });
      QObject::connect(&source, &FormPickerSharedUnderlyingModel::editorIDChanged, this, [this](const item* entry, const QString& prior) {
         auto& source = FormPickerSharedUnderlyingModel::get();
         auto& sorted = this->stubs;
         //
         int      i = sorted.indexOf(entry);
         iterator it;
         if (entry->editorID < prior) { // moving it to a spot higher in the list
            it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID < b->editorID;
            });
            int to = it - sorted.begin();
            sorted.remove(i);
            sorted.insert(to, entry);
         } else { // moving it to a spot later in the list
            it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
               return a->editorID < b->editorID;
            });
            int to = it - sorted.begin() - 1;
            sorted.remove(i);
            sorted.insert(to, entry);
         }
      });
   }

   bool FormPickerIterativeModel::_fillGrabMore() {
      if (!this->ongoing_fill.filling)
         return false;
      auto& unsorted = this->ongoing_fill.unsorted;
      //
      auto& source = FormPickerSharedUnderlyingModel::get();
      auto  start  = this->ongoing_fill.progress;
      auto  max    = source.rowCount(QModelIndex());
      auto  end    = std::min(start + make_per_tick, max);
      int   added  = 0;
      unsorted.reserve(unsorted.size() + make_per_tick);
      for (int i = start; i < end; ++i) {
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
      this->ongoing_fill.progress = end;
      return (end == max);
   }
   bool FormPickerIterativeModel::_fillSortMore() {
      if (!this->ongoing_fill.filling)
         return false;
      auto& unsorted = this->ongoing_fill.unsorted;
      auto& sorted   = this->stubs;
      if (sorted.empty())
         sorted.reserve(unsorted.size());
      int cap = std::min(sort_per_tick, unsorted.size());
      for (int i = 0; i < cap; ++i) {
         auto* entry = unsorted[i];
         if (!entry->stub) { // "NONE" special-case
            unsorted.prepend(entry);
            continue;
         }
         auto it = std::upper_bound(sorted.begin(), sorted.end(), entry, [](const item* a, const item* b) {
            if (!a->stub && b->stub)
               return true;
            if (!b->stub && a->stub)
               return false;
            return a->editorID < b->editorID;
         });
         sorted.insert(it, entry);
      }
      unsorted.remove(0, cap);
      return unsorted.empty();
   }

   void FormPickerIterativeModel::fetchMore(const QModelIndex& parent) {
      if (!this->ongoing_fill.filling)
         return;
      if (this->ongoing_fill.sorting) {
         if (this->_fillSortMore()) {
            this->ongoing_fill.timer.stop();
            this->ongoing_fill.filling  = false;
            this->ongoing_fill.sorting  = false;
            this->ongoing_fill.progress = 0;
            if (auto size = this->stubs.size()) {
               this->beginInsertRows(parent, 0, size - 1);
               this->endInsertRows();
            }
            emit filled();
         }
      } else {
         if (this->_fillGrabMore())
            this->ongoing_fill.sorting = true;
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
   #pragma endregion
}