#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "../../generic/FormsOfTypeCombobox.h"

namespace dovah {
   class form_stub;
}
class BasicFormTypeTree;

class CellListModel;
class CellListModelItem {
   friend CellListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      const dovah::form_stub* stub = nullptr;
      QString   editorID;
      form_id_t formID    = 0;
      int32_t   gridX     = 0;
      int32_t   gridY     = 0;
      //
      CellListModelItem() {}
      CellListModelItem(const dovah::form_stub*);
      //
      bool cellIsLoaded(); // TODO: update this when the render window is implemented
      void update();
};

class CellListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = CellListModelItem;
      using form_stub = dovah::form_stub;
      //
      using role_t = std::underlying_type_t<Qt::ItemDataRole>;
      static constexpr role_t SortingRole      = Qt::UserRole + 0;
      static constexpr role_t FilteringRole    = Qt::UserRole + 1;
      static constexpr role_t SortOverrideRole = Qt::UserRole + 2;
      //
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      const form_stub* worldspace = nullptr;
      //
      void insertItem(const dovah::form_stub*, bool queued);
      //
   protected slots:
      void formCreated(const dovah::form_stub*);
      void formModified(const dovah::form_stub*);
      //
   public slots:
      void clear();
      //
   public:
      CellListModel(QObject* parent = nullptr);
      ~CellListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void rebuild(const dovah::form_stub* worldspace);
      //
   signals:
      void columnCountChanged(); // needed so the widget can handle column sizes sensibly
};

class CellListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      CellListModelProxy(QObject* parent = nullptr);
      bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;
      //
      void setSortOverrideRole(Qt::ItemDataRole);
      //
   protected:
      Qt::ItemDataRole _sortOverrideRole = Qt::ItemDataRole::DisplayRole; // 1 = end (ascending); -1 = start (descending)
};

class CellList : public QTableView {
   Q_OBJECT
   public:
      CellList(QWidget* parent);
      using proxy_type      = CellListModelProxy;
      using model_type      = CellListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      //
      inline const FormsOfTypeCombobox* worldspacePicker() const noexcept { return this->_worldspaceSelector; }
      void setWorldspacePicker(const FormsOfTypeCombobox*);
      //
      dovah::bare_form_id_t formID() const noexcept;
      dovah::form_stub* formStub() const noexcept;
      //
   public slots:
      void rebuildModel();
      void clear();
      //
   signals:
      void currentCellChanged(const dovah::form_stub* cell);
      //
   protected:
      const FormsOfTypeCombobox* _worldspaceSelector = nullptr;
      //
      model_item_type* _getCurrentItem() const noexcept;
};