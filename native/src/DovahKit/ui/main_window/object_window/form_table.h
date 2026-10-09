#pragma once
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "./file_source_requirement.h"
namespace dovah {
   class form_stub;
}
namespace ui::object_window {
   class form_model_item;
   class form_table_proxy_model;
   class form_table_source_model;
}
class ObjectWindowTree;

class FormTable : public QTableView {
   Q_OBJECT;
   public:
      using model_type      = ui::object_window::form_table_source_model;
      using proxy_type      = ui::object_window::form_table_proxy_model;
      using model_item_type = ui::object_window::form_model_item;

   public:
      FormTable(QWidget* parent);
      
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      inline proxy_type* proxyModel() const noexcept {
         return (proxy_type*) this->model();
      }
      
      void setFilter(QLineEdit*);
      void setSource(ObjectWindowTree*);

      ui::object_window::file_source_requirement fileSourceRequirement() const noexcept;
      void setFileSourceRequirement(ui::object_window::file_source_requirement);

      bool onlyShowDeleted() const noexcept;
      void setOnlyShowDeleted(bool);
      
   public slots:
      void recheckFormTypes();
      void rebuildModel();
      void refilterModel(const QString&);
      void clear();
      
      void filterChanged();
      void filterFinished();
      
      void select(dovah::form_stub*);
      
   protected:
      ObjectWindowTree* _source  = nullptr;
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};