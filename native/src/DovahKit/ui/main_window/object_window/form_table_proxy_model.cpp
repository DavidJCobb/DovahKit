#include "./form_table_proxy_model.h"
#include "dovah/files/file_load_order.h"
#include "dovah/form_stub.h"
#include "./form_model_item.h"
#include "./form_table_source_model.h"

//
// KNOWN DEFECTS:
//
//  - QSortFilterProxyModel has internal mappings that it needs to build; this causes a 
//    lag spike when the user changes the Object Window's filter for the first time after 
//    files are loaded
// 
//  - QSortFilterProxyModel can also experience significant lag if a number of previously 
//    filtered-out forms cease to be filtered out. Looking at a flame graph indicates that 
//    this is due to the overhead of sorting absolutely all of those forms all at once.
//
// I think the only solution to the lag we're seeing in Debug would be to build a custom 
// sort/filter proxy model with cheaper mappings. QSortFilterProxyModel is designed to 
// support recursive filtering if you enable it (it's disabled by default, and we obviously 
// don't use it here), and as a result, its design incurs overhead for that:
// 
//  - The proxy stores its mappings as a vector of source-to-proxy row indices, a vector of 
//    the reverse, and another pair of vectors for column indices. However, the proxy is 
//    capable of storing multiple mappings keyed to different parent QModelIndexes, and it 
//    has to find the appropriate index before it can update any mapping.
// 
//  - Added branching at every filter step, for features we don't use, e.g. checking whether 
//    recursive filtering is enabled for each individual row.
// 
//  - Filter code per-column, even though we don't filter the columns.
// 
//  - We can only filter rows based on the contents of one column, or every column. What we 
//    really want is to filter just specific columns (currently indices 0 and 1).
// 
// Additionally, everything is done via virtual member functions, rather than via functions 
// that can be inlined. This includes the per-row checks. Hard to measure the overhead that 
// that adds without something to compare it to, though.
// 
// Ideally we'd make a model that stores only mappings for the root node's top-level children, 
// with compile-time options rather than run-time ones.
//

namespace ui::object_window {
   form_table_proxy_model::form_table_proxy_model(QObject* parent) : QSortFilterProxyModel(parent) {
      this->setFilterCaseSensitivity(Qt::CaseInsensitive);
      this->setFilterRole(ui::object_window::form_table_source_model::FilterableTextRole);
      this->setFilterKeyColumn(-1);
      this->setSortCaseSensitivity(Qt::CaseInsensitive);
      this->setSortRole(Qt::UserRole);

      this->setRecursiveFilteringEnabled(false);
   }

   void form_table_proxy_model::setSourceModel(QAbstractItemModel* source_model) {
      if (source_model && !qobject_cast<ui::object_window::form_table_source_model*>(source_model))
         source_model = nullptr;
      QSortFilterProxyModel::setSourceModel(source_model);
   }

   void form_table_proxy_model::setFilterInfo(const ui::object_window::filter_info& fi) {
      auto& prior = this->form_filter_info;
      if (prior == fi)
         return;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->beginFilterChange();
      #endif
      prior = fi;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
      #else
         this->invalidateFilter();
      #endif
   }
   void form_table_proxy_model::setFileSourceRequirement(ui::object_window::file_source_requirement v) {
      if (this->file_source_requirement == v)
         return;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->beginFilterChange();
      #endif
      this->file_source_requirement = v;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
      #else
         this->invalidateFilter();
      #endif
   }
   void form_table_proxy_model::setOnlyShowDeleted(bool v) {
      if (this->only_show_deleted == v)
         return;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->beginFilterChange();
      #endif
      this->only_show_deleted = v;
      #if QT_VERSION >= QT_VERSION_CHECK(6, 9, 0)
         this->endFilterChange(QSortFilterProxyModel::Direction::Rows);
      #else
         this->invalidateFilter();
      #endif
   }

   bool form_table_proxy_model::filterAcceptsStub(const dovah::form_stub* stub) const noexcept {
      if (this->only_show_deleted) {
         if (!stub->is_deleted())
            return false;
      }

      switch (this->file_source_requirement) {
         using enum ui::object_window::file_source_requirement;
         case any_files:
            break;
         case active_file_definitions:
            if (stub->source_file_count() > 1) // exists in multiple files = not defined in active file
               return false;
            if (stub->get_file_at_index(0) != stub->get_owning_load_order().get_active_file())
               return false;
            break;
         case active_file_records:
            {
               auto* active_file = stub->get_owning_load_order().get_active_file();
               if (!active_file)
                  return false;
               //
               // The active file has to be the last file in the load order; therefore, if the 
               // stub is defined or edited in the active file, the active file must be either 
               // its first source file (if defined there) or its last (if overridden there).
               //
               auto* original_file = stub->get_file_at_index(0);
               auto* winning_file  = stub->get_file_at_index(-1);
               if (original_file != active_file && winning_file != active_file)
                  return false;
            }
            break;
      }

      return this->form_filter_info.form_matches_filters(*stub);
   }
   bool form_table_proxy_model::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const {
      auto* model = (ui::object_window::form_table_source_model*)this->sourceModel();
      if (!this->form_filter_info.empty()) {
         if (auto* item = model->dataAtRow(source_row)) {
            if (!this->filterAcceptsStub(item->stub))
               return false;
         }
      }
      return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);
   }
}