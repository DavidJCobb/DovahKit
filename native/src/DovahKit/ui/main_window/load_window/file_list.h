#pragma once
#include <cstdint>
#include <vector>
#include <QAbstractItemModel>
#include <QDateTime>
#include <QString>
#include <QTableView>
#include "dovah/data/game.h"
#include "dovah/bare_form_id_t.h"

namespace dovah::tes_file_reading {
   class file_header_reader;
}

class LoadOrderFileListModel;
class LoadOrderFileListModelItem {
   friend LoadOrderFileListModel;
   public:
      using form_id_t = dovah::bare_form_id_t;
      //
      QString filename;
      bool    is_master;
      bool    selected = false;
      QString author;
      QString description;
      QVector<QString> dependencies;
      QDateTime created;
      QDateTime modified;
      //
      LoadOrderFileListModelItem() {}
      LoadOrderFileListModelItem(const dovah::tes_file_reading::file_header_reader& header, const QDateTime& created, const QDateTime& modified);
      //
      inline const QString& name() const noexcept { return this->filename; }
};

class LoadOrderFileListModel : public QAbstractTableModel {
   Q_OBJECT
   public:
      using item_type = LoadOrderFileListModelItem;
   protected:
      QVector<item_type*> children;
      item_type*  active       = nullptr;
      dovah::game current_game = dovah::game::skyrim_special;
      bool is_skyrim_classic = false;
      //
   public:
      LoadOrderFileListModel(QObject* parent = nullptr) : QAbstractTableModel(parent) {
      }
      ~LoadOrderFileListModel() {
         this->clear();
      }
      //
      QModelIndex index(int row, int column, const QModelIndex& parent) const override;
      QModelIndex parent(const QModelIndex& index) const;
      int rowCount(const QModelIndex& parent) const override;
      int columnCount(const QModelIndex& item) const override;
      Qt::ItemFlags flags(const QModelIndex& index) const override;
      QVariant data(const QModelIndex& index, int role) const override;
      bool setData(const QModelIndex& index, const QVariant& value, int role) override;
      //
      QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      //
      void clear();
      inline dovah::game game() const noexcept { return this->current_game; }
      void setGame(dovah::game);
      void insert(const dovah::tes_file_reading::file_header_reader&, const QDateTime& created, const QDateTime& modified);
      void sortByPluginsTxt();
      //
      inline const item_type* activeFile() const noexcept { return this->active; }
      inline const QVector<item_type*>& files() const noexcept { return this->children; }
      //
      void setActiveFile(item_type*) noexcept;
      void setSelected(item_type*, bool) noexcept;
      void toggleSelected(item_type*) noexcept;
};

class LoadOrderFileList : public QTableView {
   Q_OBJECT
   public:
      LoadOrderFileList(QWidget* parent);
      using model_type      = LoadOrderFileListModel;
      using model_item_type = model_type::item_type;
      //
      inline model_type* unwrappedModel() const noexcept {
         return (model_type*)this->model();
      }
      //
      inline dovah::game game() const noexcept {
         auto model = this->unwrappedModel();
         if (model)
            return model->game();
         return dovah::game::skyrim_special;
      }
      //
   public slots:
      void listFiles(dovah::game);
};