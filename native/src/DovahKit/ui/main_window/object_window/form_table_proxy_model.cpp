#include "./form_table_proxy_model.h"
#include "dovah/files/file_load_order.h"
#include "dovah/form_stub.h"
#include "./form_model_item.h"
#include "./form_table_source_model.h"

namespace {
   constexpr const Qt::CaseSensitivity case_sensitivity_for_sorting = Qt::CaseInsensitive;
}

namespace ui::object_window {
   form_table_proxy_model::form_table_proxy_model(QObject* parent) : QSortFilterProxyModel(parent) {
      this->setFilterCaseSensitivity(Qt::CaseInsensitive);
      this->setFilterRole(ui::object_window::form_table_source_model::FilterableTextRole);
      this->setFilterKeyColumn(-1);
      this->setSortCaseSensitivity(case_sensitivity_for_sorting);
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
   /*virtual*/ bool form_table_proxy_model::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const /*override*/ {
      //
      // This is all *basically* what QSortFilterProxyModel already does on its own, but we're 
      // accessing the model data more directly, so there's a little less indirection. Sorting 
      // is the heaviest part of the proxy model per my perf tests, so every little bit helps.
      //
      const auto* src_item_l = form_table_source_model::data_for_qmi(source_left);
      const auto* src_item_r = form_table_source_model::data_for_qmi(source_right);
      if (!src_item_l)
         return true;
      if (!src_item_r)
         return false;
      switch (source_left.column()) {
         case form_table_source_model::Column::EditorID:
            return src_item_l->editor_id.compare(src_item_r->editor_id, case_sensitivity_for_sorting) < 0;
         case form_table_source_model::Column::FormID:
            return src_item_l->form_id < src_item_r->form_id;
         case form_table_source_model::Column::UserCount:
            return src_item_l->user_count < src_item_r->user_count;
      }
      return false;
   }
}