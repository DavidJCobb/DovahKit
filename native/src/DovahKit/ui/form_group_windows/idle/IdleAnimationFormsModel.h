#pragma once
#include <exception>
#include <unordered_map>
#include <vector>
#include <QAbstractItemModel>
#include "dovah/datastores/idles.h"
namespace dovah {
   class form_stub;
}
namespace ui::types {
   class game_file_path;
}

class IdleAnimationFormsModel : public QAbstractItemModel {
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
      static constexpr const Qt::ItemDataRole FormStubRole = (Qt::ItemDataRole)(Qt::ItemDataRole::UserRole + 1);

      static constexpr const size_t ColumnCount = 1;

      class too_many_to_duplicate_exception : public std::exception {
         public:
            too_many_to_duplicate_exception(size_t needed, size_t available)
            :
               std::exception("Insufficient form IDs available to duplicate this idle"),
               form_id_counts({needed, available})
            {};

            const struct {
               size_t needed;
               size_t available;
            } form_id_counts;
      };

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

      #pragma region Node/QMI utils and node lookups
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

      #pragma region Constraints
         bool _can_create_new_idle_in(const idle_parent_node&) const;
         bool _can_ever_duplicate(const idle_node&) const;
      #pragma endregion

      static dovah::file_load_order* _get_file_load_order();

      #pragma region Form events
         void _on_game_data_acquired();
         void _on_game_data_abandon();
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deletion_imminent(dovah::form_stub&, bool just_being_flagged);
         void _on_form_deleted(uint32_t form_id, bool just_being_flagged);
      #pragma endregion

      void _rebuild_datastore();
      void _recache_action(const action_node&);
      void _recache_action(const dovah::form_stub&);
      void _recache_idle(const idle_node&);
      void _recache_idle(const dovah::form_stub&);

      #pragma region Form utils
         dovah::form_stub* _try_silently_create_idle(QString editor_id) noexcept(false);
         dovah::form_stub* _try_silently_duplicate_idle(dovah::form_stub& idle) noexcept(false);
      #pragma endregion
      #pragma region Node utils
         action_node& _get_or_create_action(graph_node&, dovah::form_stub& action);
         idle_node* _create_action_root(graph_node&, dovah::form_stub& action, QString idle_editor_id) noexcept(false);
      #pragma endregion

   public:
      IdleAnimationFormsModel(QObject* parent = nullptr);

      #pragma region Accessors
         QModelIndex graphQMI(QString path) const noexcept;
         QModelIndex idleQMI(dovah::form_stub&) const noexcept;

         QModelIndex getOrCreateGraph(QString path);

      protected:
         [[nodiscard]] std::vector<dovah::form_stub*> _actionsByGraph(const graph_node&) const noexcept;
      public:
         [[nodiscard]] std::vector<dovah::form_stub*> actionsByGraph(const QModelIndex&) const noexcept;
         [[nodiscard]] std::vector<dovah::form_stub*> actionsByGraph(QString path) const noexcept;

         QModelIndex createActionRoot(const QModelIndex& graph_qmi, dovah::form_stub& action, QString idle_editor_id) noexcept(false);
         QModelIndex createActionRoot(QString graph_path, dovah::form_stub& action, QString idle_editor_id) noexcept(false);

         bool canCreateIdleIn(const QModelIndex& parent) const;
         QModelIndex createIdle(const QModelIndex& parent, QString idle_editor_id) noexcept(false); // may throw `dovah::exceptions::form_creation_failed`

         bool canEverDuplicateIdle(const QModelIndex&) const;
         QModelIndex duplicateIdle(const QModelIndex&, bool and_descendants) noexcept(false); // may throw `too_many_to_duplicate_exception` or `dovah::exceptions::form_creation_failed`

         bool canDeleteIdle(const QModelIndex&) const;
         void deleteIdle(const QModelIndex&, QWidget* error_dialog_parent = nullptr); // deletes the idle *and its descendants*
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
         bool last_node_placement_was_an_insertion = false;
      } _callback_state;
};