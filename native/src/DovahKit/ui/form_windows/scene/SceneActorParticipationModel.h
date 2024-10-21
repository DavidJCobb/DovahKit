#pragma once
#include <vector>
#include <QAbstractItemModel>

namespace SceneFormVisualEditor_impl {
   class Actor;
}

class SceneActorParticipationModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      struct Column {
         Column() = delete;
         enum {
            NoPlayerDialogue,
            Optional,

            _COUNT
         };
      };
      static constexpr const size_t ColumnCount = Column::_COUNT;

   public:
      void addActor(const SceneFormVisualEditor_impl::Actor&);
      void commitActor(SceneFormVisualEditor_impl::Actor&) const;
      
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

   protected:
      struct BehaviorFlags {
         bool pause = false;
         bool end   = false;
      };

      struct Actor {
         uint32_t alias_id = -1;
         QString  name;
         struct {
            bool no_player_activation = false;
            bool optional = false;
         } flags;
      };
      
      std::vector<Actor> _data;
};