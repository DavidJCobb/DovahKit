#pragma once
#include <QAbstractItemModel>
#include "./QuestAllDialogueDatastore.h"

class QuestDialogueBranchedTopicsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            EditorID,
            IsBranchStartingTopic,
            FormID,
            Priority,
            DisplayText,

            __COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::__COUNT;

      static constexpr const Qt::ItemDataRole FormStubRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 0);

   protected:
      using datastore_type = QuestAllDialogueDatastore;
      using container_type = datastore_type::Branch;
      using node_type      = datastore_type::Topic;
      
      QPointer<datastore_type>      _datastore;
      const container_type*         _root = nullptr;
      std::vector<const node_type*> _data;

   public:
      QuestDialogueBranchedTopicsModel(QObject* parent = nullptr);

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
      void setRootBranch(dovah::form_stub*);

   protected:
      void _fill();
      void _on_node_edited(const node_type&);
      void _re_sort_node(const node_type&);
      decltype(_data)::iterator _insertion_point_for(const node_type& item);
      
};