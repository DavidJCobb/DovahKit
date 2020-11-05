#pragma once
#include <cstdint>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QString>
#include <QTableView>
#include <QTimer>
#include "../../../dovah/core.h"
#include "../../../dovah/data/game_settings.h"

namespace dovah {
   class loaded_game_setting;
}

class GameSettingListModel;
class GameSettingListModelItem {
   //
   // Given a model which displays all users of a form, this item represents a 
   // user form (as opposed to the used form).
   //
   friend GameSettingListModel;
   public:
      using bare_form_id_t = dovah::bare_form_id_t;
      //
      QString name;
      QString description;
      QString last_file;
      bare_form_id_t formID = 0;
      bool    is_in_active_file = false;
      struct {
         bool    boolean = false;
         QString string;
         int32_t number  = 0;
         float   float32 = 0.0F;
      } value;
      dovah::game_setting_type type = dovah::game_setting_type::none;
      //
      GameSettingListModelItem() {}
      GameSettingListModelItem(const dovah::loaded_game_setting&);
      GameSettingListModelItem(const dovah::game_setting_definition&);
      void updateFrom(const dovah::loaded_game_setting&);
      //
      QString valueAsString() const noexcept;
};

class GameSettingListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = GameSettingListModelItem;
      static constexpr int ColumnName   = 0;
      static constexpr int ColumnValue  = 1;
      static constexpr int ColumnFormID = 2;
      static constexpr int ColumnFile   = 3;
      static constexpr Qt::ItemDataRole SortRole   = (Qt::ItemDataRole)(Qt::UserRole + 0);
      static constexpr Qt::ItemDataRole FilterRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
   protected:
      QVector<item_type*> children;
      QVector<item_type*> queued_additions;
      //
   protected slots:
      void gameSettingValueChanged(const char* name);
      //
   public:
      GameSettingListModel(QObject* parent = nullptr);
      ~GameSettingListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      inline const item_type* row(int rowIndex) const noexcept;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void build();
      //
   public slots:
      void clear();
};

class GameSettingListModelProxy : public QSortFilterProxyModel {
   Q_OBJECT
   public:
      GameSettingListModelProxy(QObject* parent = nullptr);
      using model_type      = GameSettingListModel;
      using model_item_type = model_type::item_type;
};

class GameSettingList : public QTableView {
   Q_OBJECT
   public:
      GameSettingList(QWidget* parent);
      using model_type        = GameSettingListModel;
      using model_item_type   = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         auto wrapper = (QSortFilterProxyModel*)this->model();
         return wrapper ? (model_type*)wrapper->sourceModel() : nullptr;
      }
      void setTextFilter(QLineEdit*);
      void setFormTypeFilter(dovah::form_type_t);
      //
   public slots:
      void build();
      void refilterModelByText(const QString&);
      void textFilterChanged();
      void textFilterFinished();
      //
   protected:
      QLineEdit* _filter         = nullptr;
      QTimer*    _filterThrottle = new QTimer(this);
};