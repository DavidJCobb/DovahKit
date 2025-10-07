#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "./object_window_treeview.h"
#include "./filter_info.h"

namespace dovah {
   class form_stub;
}

namespace ui::object_window {
   struct filter_info;
}

class FormTableModel;
class FormTableModelItem {
   friend FormTableModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      dovah::form_stub* stub = nullptr;
      QString   editorID;
      form_id_t formID    = 0;
      uint32_t  userCount = 0;
      //
      bool is_active   = false;
      bool is_injected = false;
      bool is_none     = false;
      //
      FormTableModelItem() {}
      FormTableModelItem(dovah::form_stub*);
      //
      inline const QString& name() const noexcept { return this->editorID; }
      void update();
      bool updateUserCount(); // returns true if any changes were made
};

class FormTableModel final : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = FormTableModelItem;
      using form_stub = dovah::form_stub;
      using form_type_set = QVector<dovah::form_type>;
      //
      static constexpr const Qt::ItemDataRole RawDataRole        = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole);
      static constexpr const Qt::ItemDataRole FilterableTextRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1);
      //
   protected:
      form_type_set       form_types; // list of all form types that the Object Window should be capable of displaying under any circumstance
      QVector<item_type*> children;
      QVector<item_type*> pending_additions;
      QVector<form_stub*> forms_pending_use_info_update;
      //
      void doUseInfoUpdate();
      void insertItem(form_stub*, bool queued);
      //
   protected slots:
      void formCreated(form_stub*);
      void formModified(const form_stub*);
      void formModificationImminent(const form_stub*);
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      //
   public slots:
      void clear();
      //
   public:
      FormTableModel(QObject* parent = nullptr);
      ~FormTableModel() {
         this->clear();
      }
      //
      QModelIndex index(dovah::form_stub*) const;
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      QMimeData* mimeData(const QModelIndexList& indexes) const;
      virtual QStringList mimeTypes() const override;
      //
      void rebuild();
      void setBaseFormTypes(const form_type_set&);
      //
      const item_type* dataAtRow(int) const noexcept;
};

class FormTableModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      FormTableModelProxy(QObject* parent = nullptr) : QSortFilterProxyModel(parent) {
         this->setFilterCaseSensitivity(Qt::CaseInsensitive);
         this->setFilterRole(FormTableModel::FilterableTextRole);
         this->setFilterKeyColumn(-1);
         this->setSortCaseSensitivity(Qt::CaseInsensitive);
         this->setSortRole(Qt::UserRole);
      }

      virtual void setSourceModel(QAbstractItemModel* sourceModel) override;

      constexpr const ui::object_window::filter_info& filterInfo() const noexcept { return this->form_filter_info; }
      void setFilterInfo(const ui::object_window::filter_info&);

   protected:
      ui::object_window::filter_info form_filter_info;

      bool filterAcceptsStub(const dovah::form_stub* stub) const noexcept;
      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};

class FormTable : public QTableView {
   Q_OBJECT
   public:
      FormTable(QWidget* parent);
      using model_type      = FormTableModel;
      using proxy_type      = FormTableModelProxy;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      inline proxy_type* proxyModel() const noexcept {
         return (proxy_type*) this->model();
      }
      //
      void setFilter(QLineEdit*);
      void setSource(ObjectWindowTree*);
      //
   public slots:
      void recheckFormTypes();
      void rebuildModel();
      void refilterModel(const QString&);
      void clear();
      //
      void filterChanged();
      void filterFinished();
      //
      void select(dovah::form_stub*);
      //
   protected:
      ObjectWindowTree* _source  = nullptr;
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};