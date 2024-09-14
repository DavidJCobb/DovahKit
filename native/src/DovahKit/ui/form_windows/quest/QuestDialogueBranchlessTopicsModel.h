#pragma once
#include <QAbstractItemModel>
#include <QPointer>
#include "./QuestAllDialogueDatastore.h"
#include "dovah/data/dialogue/category.h"

class QuestDialogueBranchlessTopicsModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            EditorID,
            Subtype,
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
      using node_type      = datastore_type::Topic;
      
      dovah::dialogue::category     _category = dovah::dialogue::category::topic;
      QPointer<datastore_type>      _datastore;
      std::vector<const node_type*> _data;

   public:
      QuestDialogueBranchlessTopicsModel(QObject* parent = nullptr);

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
      void setCategory(dovah::dialogue::category);
      void setDatastore(datastore_type*);

      const node_type* node(size_t row) const;
      size_t index_of(const dovah::form_stub&) const;

   protected:
      void _fill();
      void _on_node_edited(const node_type&);
      void _re_sort_node(const node_type&);
      decltype(_data)::iterator _insertion_point_for(const node_type& item);

      QString _subtype_name(uint32_t subtype_signature) const;

      void _update_topic_subtype_name(const char* game_setting_name);
      void _update_all_topic_subtype_names();
};