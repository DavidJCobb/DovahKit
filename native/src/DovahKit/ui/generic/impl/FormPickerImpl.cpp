#include "FormPickerImpl.h"
#include "../../../editor/core.h"
#include "../../../editor/form_stub_meta_type.h"

/*

   ADVANTAGES

    - No lag when populating the form dropdown, as we populate it 500 items per tick across multiple 
      ticks. This means that even statics, which contain thousands of forms, can be listed quickly.

   DEFECTS

    - The form combobox becomes greyed out while it is populating. This is because we disable it 
      whenever it's empty, as a shortcut to disabling it whenever there are no forms available. This 
      is arguably correct behavior, but it's a bit janky; it'd be nice if we could show some sort of 
      progress bar animation on top of the combobox instead.

    - Heavy lag time when opening the form dropdown for the first time, if there are especially many 
      forms inside (e.g. STAT). This resets every time we repopulate the dropdown, so changing from 
      STAT to SNDR and back to STAT will cause the lag to happen the next time the dropdown opens.
      
       - Alleviated slightly by making the QComboBox's internal QListView use uniform item sizes.

       - I suspect this overhead comes from QSortFilterProxyModel sorting the combobox.

          - We already use FormPickerIterativeModel to do filtering. Perhaps we should have it sort 
            the items across multiple ticks as well and just remove QSortFilterProxyModel from the 
            picture entirely. Easiest way would be to have it initially add stubs to a secondary 
            list, (ongoing_fill.unsorted), and then have it transfer stubs to the final list.

             - FormPickerIterativeModel needs to react to forms being removed from the underlying 
               "all stubs" model. Currently, we rely on a nice hack where we can check whether the 
               removed stub was above our "progress" value. When sorting, we'll want to check 
               whether the removed (first) value was below the size of the sorted list; if not, 
               then we can quickly pluck it out of the unsorted list.

               Essentially, if (first > sorted_list.size()), then we can just remove the elements 
               starting from (unsorted_list[first - sorted_list.size()); otherwise, we have to 
               actually search the sorted list for the elements to remove. And of course, since 
               we can potentially be talking about a range, we'll have to be careful to handle 
               the case of the range stretching across both lists.

               This is all concerning the QAbstractItemModel::rowsRemoved signal/slot, I mean.

                - Additionally, we can better handle the case of the entire model being emptied 
                  (i.e. all data unloaded) if we check whether the number of removed elements is 
                  equal to the size of the sorted list (possibly minus 1 if we allow none).

   MISCELLANEOUS

    - Investigate having FormPickerIterativeModel build more than 500 items per tick.

    - Investigate how much of the list we can have FormPickerIterativeModel sort in a single tick.

    - Consider renaming FormPickerIterativeModel to FormPickerSortFilterModel, and adding documentation 
      comments which describe it as "filtering and sorting across time."
 
*/

namespace {
   bool _should_exclude_form_type(dovah::form_type_t ft) {
      if (dovah::form_type_info::form_type_is_reference(ft))
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
}

namespace FormPickerImpl {
   #pragma region FormPickerSharedUnderlyingModel
   FormPickerSharedUnderlyingModel::FormPickerSharedUnderlyingModel() : QAbstractItemModel(&DovahKitCore::get()) {
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
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
         this->all_stubs.reserve(count);
         //
         for (const auto& info : dovah::form_types) {
            if (_should_exclude_form_type(info.formType))
               continue;
            editor.for_each_form_of_type(info.formType, [this](dovah::form_stub* stub) {
               if (_should_exclude_form(stub))
                  return false;
               this->all_stubs.push_back(stub);
               return false;
            });
         }
         this->endResetModel();
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         this->beginResetModel();
         this->all_stubs = { nullptr };
         this->endResetModel();
      });
      QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
         auto index = this->all_stubs.indexOf(stub);
         if (index < 0)
            return;
         this->beginRemoveRows(QModelIndex(), index, index);
         this->all_stubs.remove(index);
         this->endRemoveRows();
      });
      QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
         auto i = this->all_stubs.size();
         this->beginInsertRows(QModelIndex(), i, i);
         this->all_stubs.push_back(stub);
         this->endInsertRows();
      });
      QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
         auto i = this->all_stubs.indexOf(stub);
         if (i < 0)
            return;
         auto index = this->index(i, 0, QModelIndex());
         emit dataChanged(index, index, { Qt::DisplayRole, Qt::ToolTipRole }); // in case the editor ID changed
      });
      QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
         auto i = this->all_stubs.indexOf(stub);
         if (i < 0)
            return;
         auto index = this->index(i, 0, QModelIndex());
         emit dataChanged(index, index, { FormIDRole });
      });
   }

   QModelIndex FormPickerSharedUnderlyingModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->hasIndex(row, column, parent))
         return QModelIndex();
      dovah::form_stub* childItem = this->all_stubs.value(row);
      if (childItem)
         return this->createIndex(row, column, childItem);
      return QModelIndex();
   }
   QModelIndex FormPickerSharedUnderlyingModel::parent(const QModelIndex& index) const {
      return QModelIndex();
   }
   int FormPickerSharedUnderlyingModel::rowCount(const QModelIndex& parent) const {
      return this->all_stubs.size();
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
      auto* stub = (dovah::form_stub*) index.internalPointer();
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
            return stub->get_editor_id();
      }
      return QVariant();
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
         auto& source = FormPickerSharedUnderlyingModel::get();
         if (this->ongoing_fill.filling) {
            if (first < this->ongoing_fill.progress) {
               auto cap = last + 1;
               if (cap > this->ongoing_fill.progress)
                  cap = this->ongoing_fill.progress;
               for (int i = first; i < cap; ++i) {
                  auto  index = source.index(i, 0, parent);
                  auto  data  = source.data(index, FormPickerSharedUnderlyingModel::FormStubRole);
                  if (!data.isValid())
                     continue;
                  auto* stub = data.value<dovah::form_stub*>();
                  this->stubs.push_back(stub);
               }
            }
            return;
         }
         for (int i = first; i <= last; ++i) {
            auto  index = source.index(i, 0, parent);
            auto  data  = source.data(index, FormPickerSharedUnderlyingModel::FormStubRole);
            if (!data.isValid())
               continue;
            auto* stub = data.value<dovah::form_stub*>();
            this->stubs.push_back(stub);
         }
      });
      QObject::connect(&source, &QAbstractItemModel::rowsRemoved, this, [this](const QModelIndex& parent, int first, int last) {
         auto& source = FormPickerSharedUnderlyingModel::get();
         int cap = last;
         if (this->ongoing_fill.filling) {
            if (first >= this->ongoing_fill.progress)
               return;
            cap = last + 1;
            if (cap > this->ongoing_fill.progress)
               cap = this->ongoing_fill.progress;
         }
         for (int i = first; i <= cap; ++i) {
            auto  index = source.index(i, 0, parent);
            auto  data  = source.data(index, FormPickerSharedUnderlyingModel::FormStubRole);
            auto* stub  = data.value<dovah::form_stub*>();
            if (stub) {
               int index = this->stubs.indexOf(stub);
               if (index >= 0)
                  this->stubs.remove(index);
            }
         }
      });
   }

   void FormPickerIterativeModel::fetchMore(const QModelIndex& parent) {
      if (!this->ongoing_fill.filling)
         return;
      if (this->ongoing_fill.allow_none && this->stubs.empty())
         this->stubs.push_back(nullptr);
      //
      auto  dummy  = QModelIndex();
      auto& source = FormPickerSharedUnderlyingModel::get();
      auto  start  = this->ongoing_fill.progress;
      auto  max    = source.rowCount(dummy);
      auto  end    = std::min(start + 500, max);
      int   added  = 0;
      this->stubs.reserve(this->stubs.size() + 500);
      for (int i = start; i < end; ++i) {
         auto  index = source.index(i, 0, dummy);
         auto  data  = source.data(index, FormPickerSharedUnderlyingModel::FormStubRole);
         if (!data.isValid())
            continue;
         auto* stub  = data.value<dovah::form_stub*>();
         if (stub && !this->ongoing_fill.form_types.isEmpty() && !this->ongoing_fill.form_types.contains(stub->formType))
            continue;
         this->stubs.push_back(stub);
      }
      this->ongoing_fill.progress = end;
      if (end == max) {
         this->ongoing_fill.timer.stop();
         this->ongoing_fill.filling = false;
         if (auto size = this->stubs.size()) {
            this->beginInsertRows(parent, 0, size - 1);
            this->endInsertRows();
         }
      }
   }
   bool FormPickerIterativeModel::canFetchMore(const QModelIndex& parent) const {
      if (!this->ongoing_fill.filling)
         return false;
      return this->ongoing_fill.progress < FormPickerSharedUnderlyingModel::get().rowCount(parent);
   }

   void FormPickerIterativeModel::refill(bool allow_none, const QVector<dovah::form_type_t>& form_types) {
      this->ongoing_fill.filling = true;
      this->stubs.clear();
      this->ongoing_fill.allow_none = allow_none;
      this->ongoing_fill.form_types = form_types;
      this->ongoing_fill.progress   = 0;
      this->ongoing_fill.timer.start();
   }
   
   QModelIndex FormPickerIterativeModel::index(int row, int column, const QModelIndex& parent) const {
      if (!this->hasIndex(row, column, parent))
         return QModelIndex();
      dovah::form_stub* childItem = this->stubs.value(row);
      if (childItem)
         return this->createIndex(row, column, childItem);
      return QModelIndex();
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
      auto* stub = (dovah::form_stub*) index.internalPointer();
      switch (role) {
         case FormPickerSharedUnderlyingModel::FormIDRole:
            if (!stub)
               return 0;
            return stub->formID;
         case FormPickerSharedUnderlyingModel::FormStubRole:
            return QVariant::fromValue(stub);
         case Qt::DisplayRole:
         case Qt::ToolTipRole:
            if (!stub)
               return tr("NONE");
            return stub->get_editor_id();
      }
      return QVariant();
   }
   #pragma endregion

   #pragma region FormPickerProxyModel
   FormPickerProxyModel::FormPickerProxyModel(QObject* parent) : QSortFilterProxyModel(parent) {
      QSortFilterProxyModel::setSourceModel(new FormPickerIterativeModel(this));
      this->setSortCaseSensitivity(Qt::CaseInsensitive);
      this->sort(0);
   }

   void FormPickerProxyModel::setAllowedFormTypes(const QVector<dovah::form_type_t>& v) {
      this->_formTypes = v;
      ((FormPickerIterativeModel*)this->sourceModel())->refill(this->_allowNone, this->_formTypes);
      this->invalidateFilter();
   }
   void FormPickerProxyModel::setAllowNone(bool b) {
      if (this->_allowNone == b)
         return;
      this->_allowNone = b;
      ((FormPickerIterativeModel*)this->sourceModel())->refill(this->_allowNone, this->_formTypes);
      this->invalidateFilter();
   }
   void FormPickerProxyModel::updateParameters(bool allow_none, const QVector<dovah::form_type_t>& form_types) {
      this->_allowNone = allow_none;
      this->_formTypes = form_types;
      ((FormPickerIterativeModel*)this->sourceModel())->refill(this->_allowNone, this->_formTypes);
      this->invalidateFilter();
   }

   bool FormPickerProxyModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
      auto        source = this->sourceModel();
      QModelIndex index  = source->index(source_row, 0, source_parent);
      if (!index.isValid())
         return false;
      auto data = source->data(index, FormPickerSharedUnderlyingModel::FormStubRole);
      if (!data.isValid())
         return false;
      auto* stub = data.value<dovah::form_stub*>();
      if (!stub)
         return this->_allowNone;
      if (this->_formTypes.isEmpty())
         return true;
      return this->_formTypes.contains(stub->formType);
   }
   bool FormPickerProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
      auto source = this->sourceModel();
      auto a      = source->data(left,  FormPickerSharedUnderlyingModel::FormIDRole).toInt();
      auto b      = source->data(right, FormPickerSharedUnderlyingModel::FormIDRole).toInt();
      if ((a & b) == 0) { // is either of them zero?
         if (!a)
            return true;
         if (!b)
            return false;
      }
      return QSortFilterProxyModel::lessThan(left, right);
   }
   #pragma endregion
}