#pragma once
#include <QSortFilterProxyModel>
#include "./file_source_requirement.h"
#include "./filter_info.h"

namespace ui::object_window {
   class form_table_proxy_model : public QSortFilterProxyModel {
      Q_OBJECT;
      public:
         form_table_proxy_model(QObject* parent = nullptr);

         virtual void setSourceModel(QAbstractItemModel* sourceModel) override;

         constexpr const ui::object_window::filter_info& filterInfo() const noexcept {
            return this->form_filter_info;
         }
         void setFilterInfo(const ui::object_window::filter_info&);

         constexpr ui::object_window::file_source_requirement fileSourceRequirement() const noexcept {
            return this->file_source_requirement;
         }
         void setFileSourceRequirement(ui::object_window::file_source_requirement);

         constexpr bool onlyShowDeleted() const noexcept {
            return this->only_show_deleted;
         }
         void setOnlyShowDeleted(bool);

      protected:
         ui::object_window::file_source_requirement file_source_requirement = ui::object_window::file_source_requirement::any_files;
         ui::object_window::filter_info             form_filter_info;
         bool only_show_deleted = false;

         bool filterAcceptsStub(const dovah::form_stub* stub) const noexcept;
         virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
         virtual bool lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const override;
   };
}