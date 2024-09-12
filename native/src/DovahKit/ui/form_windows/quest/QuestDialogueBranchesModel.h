#pragma once
#include <QAbstractItemModel>
#include "./QuestAllDialogueDatastore.h"

class QuestDialogueBranchesModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            EditorID,
            FormID,
            Flags, // B for blocking, T for top-level, E for exclusive; multiple allowed e.g. BE

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole FormStubRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 0);

   protected:
      using datastore_type = QuestAllDialogueDatastore;
      using node_type      = datastore_type::Branch;
      
      QPointer<datastore_type>      _datastore;
      std::vector<const node_type*> _data;

   public:
      QuestDialogueBranchesModel(QObject* parent = nullptr);

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const override;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent = {}) const override;
            virtual int         columnCount(const QModelIndex& parent = {}) const override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
         virtual QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
      #pragma endregion

      inline datastore_type* datastore() const { return this->_datastore; }
      void setDatastore(datastore_type*);

   protected:
      void _fill();
      void _on_node_edited(const node_type&);
      void _re_sort_node(const node_type&);
      decltype(_data)::iterator _insertion_point_for(const node_type& item);
      
};