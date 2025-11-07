#pragma once
#include <memory>
#include <unordered_map>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/datastores/idles.h"
#include "./IdleAnimationFormsModel_impl/_base_node.h"
#include "./IdleAnimationFormsModel_impl/graph_node.h"
#include "./IdleAnimationFormsModel_impl/action_parent_node.h"
#include "./IdleAnimationFormsModel_impl/idle_parent_node.h"
namespace dovah {
   class form_stub;
}
namespace IdleAnimationFormsModel_impl {
   class action_node;
   class idle_node;
}

class IdleAnimationFormsModel_2 : public QAbstractItemModel {
   Q_OBJECT;
   public:
      enum class NodeType {
         Graph,
         Action,
         Idle,
         LooseIdlesPerGraph,
         LooseActionsPerModel,
         LooseIdlesPerModel,
      };
      Q_ENUM(NodeType);

      static constexpr const Qt::ItemDataRole NodeTypeRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole);

   protected:
      using datastore_type = dovah::datastores::idles;
      using datastore_node = datastore_type::node;

      using action_node = datastore_type::action_node;
      using graph_node  = datastore_type::graph_node;
      using idle_node   = datastore_type::idle_node;
      //
      using action_parent_node = datastore_type::action_parent_node;
      using idle_parent_node   = datastore_type::idle_parent_node;

      struct node_cached_data {
         QString display_string;
      };

      #pragma region Node utils
         QModelIndex _qmi_for_model_root() const;
         QModelIndex _qmi_for_node(const datastore_node&, int column = 0) const;
         QModelIndex _qmi_for_child_node(int row, int column, const datastore_node& parent) const;
         bool _qmi_is_child_of(const QModelIndex&, const datastore_node&) const;

         const datastore_node* _child_node_by_row(const datastore_node* parent, size_t row) const;

         const datastore_node* _node_for_qmi(const QModelIndex&) const;
         datastore_node* _node_for_qmi(const QModelIndex&);

         const datastore_node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row) const;
         datastore_node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row);

         const graph_node* _node_for_graph_path(QString) const;
         graph_node* _node_for_graph_path(QString);

         action_node* _node_for_action(graph_node*, const dovah::form_stub&);
         action_node* _node_for_action(graph_node&, const dovah::form_stub&);
         action_node* _node_for_loose_action(const dovah::form_stub&);
      #pragma endregion

      #pragma region Form events
         void _on_game_data_acquired();
         void _on_game_data_abandon();
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deletion_imminent(dovah::form_stub&, bool just_being_flagged);
      #pragma endregion

      void _rebuild_datastore();
      void _recache_action(const action_node&);
      void _recache_action(const dovah::form_stub&);
      void _recache_idle(const idle_node&);
      void _recache_idle(const dovah::form_stub&);

   public:
      IdleAnimationFormsModel_2(QObject* parent = nullptr);

      #pragma region Accessors
         QModelIndex graphQMI(QString path) const noexcept;

      protected:
         [[nodiscard]] std::vector<dovah::form_stub*> _actionsByGraph(const graph_node&) const noexcept;
      public:
         [[nodiscard]] std::vector<dovah::form_stub*> actionsByGraph(const QModelIndex&) const noexcept;
         [[nodiscard]] std::vector<dovah::form_stub*> actionsByGraph(QString path) const noexcept;

      protected:
         QModelIndex _createActionRoot(graph_node&, dovah::form_stub& action, QString idle_editor_id);
      public:
         QModelIndex createActionRoot(const QModelIndex& graph_qmi, dovah::form_stub& action, QString idle_editor_id);
         QModelIndex createActionRoot(QString graph_path, dovah::form_stub& action, QString idle_editor_id);
      #pragma endregion

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
         #pragma region Drag and drop
            #pragma region Whole-model queries
               virtual QStringList mimeTypes() const override;
               virtual Qt::DropActions supportedDropActions() const override;
            #pragma endregion
            /*//
            virtual QMimeData* mimeData(const QModelIndexList&) const override;
            virtual bool canDropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) const override;
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            //*/
         #pragma endregion
      #pragma endregion

   protected:
      datastore_type _datastore;
      std::unordered_map<datastore_node*, node_cached_data> _cache;
      struct {
         bool ignore_next_created_idle = false;
         bool emitted_last_deletion = false;
      } _callback_state;
};