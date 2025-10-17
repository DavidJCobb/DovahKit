#pragma once
#include <memory>
#include <vector>
#include <QAbstractItemModel>
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

class IdleAnimationFormsModel : public QAbstractItemModel {
   Q_OBJECT;
   protected:
      using node      = IdleAnimationFormsModel_impl::node;
      using node_type = IdleAnimationFormsModel_impl::node_type;
      //
      using action_parent_node = IdleAnimationFormsModel_impl::action_parent_node;
      using idle_parent_node = IdleAnimationFormsModel_impl::idle_parent_node;
      //
      using action_node = IdleAnimationFormsModel_impl::action_node;
      using graph_node  = IdleAnimationFormsModel_impl::graph_node;
      using idle_node   = IdleAnimationFormsModel_impl::idle_node;
      //
      using loose_action_parent_node = IdleAnimationFormsModel_impl::loose_action_parent_node;
      using loose_idle_parent_node   = IdleAnimationFormsModel_impl::loose_idle_parent_node;
      //
      template<typename T>
      using node_unique_ptr = IdleAnimationFormsModel_impl::node_unique_ptr<T>;
      using node_unique_deleter = IdleAnimationFormsModel_impl::node_deleter;

      #pragma region Node utils
         QModelIndex _qmi_for_model_root() const;
         QModelIndex _qmi_for_node(const node&, int column = 0) const;
         QModelIndex _qmi_for_child_node(int row, int column, const node& parent) const;

         const node* _child_node_by_row(const node* parent, size_t row) const;

         const node* _node_for_qmi(const QModelIndex&) const;
         node* _node_for_qmi(const QModelIndex&);

         const node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row) const;
         node* _child_node_for_qmi(const QModelIndex& parent_qmi, int row);

         const graph_node* _node_for_graph_path(QString) const;
         graph_node* _node_for_graph_path(QString);

         action_node* _node_for_action(const graph_node*, const dovah::form_stub&);
         action_node* _node_for_action(const graph_node&, const dovah::form_stub&);
         action_node* _node_for_loose_action(const dovah::form_stub&);
      #pragma endregion

      // If you wish to re-parent an idle node, you must also update the wrapped IDLE form 
      // by calling this function.
      void _update_idle_node_parent(idle_node&);

      #pragma region Form events
         void _on_game_data_acquired();
         void _on_game_data_abandon();
         void _on_form_created(dovah::form_stub&);
         void _on_form_modified(dovah::form_stub&);
         void _on_form_deletion_imminent(dovah::form_stub&, bool just_being_flagged);
      #pragma endregion

      // Will return nullptr if the path is empty.
      graph_node* _get_or_emplace_graph(QString path, bool emit_signals_for_emplace);
      void _insert_idle_into(node_unique_ptr<idle_node>&&, idle_parent_node&, bool emit_signals);
      void _reparent_child_idles(
         idle_parent_node&  parent_prior,
         idle_parent_node&  parent_after,
         const QModelIndex& parent_prior_qmi,
         const QModelIndex& parent_after_qmi
      );

      action_node* _import_action(action_parent_node&, dovah::form_stub&, bool emit_signals);

   public:
      IdleAnimationFormsModel(QObject* parent = nullptr);

      void import_idle(dovah::form_stub& idle, bool emit_signals);
      void import_all_idles();

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
      struct {
         std::vector<node_unique_ptr<graph_node>> graphs;
         struct {
            node_unique_ptr<loose_action_parent_node> actions;
            node_unique_ptr<loose_idle_parent_node> idles;
         } loose;
      } nodes;
      std::unordered_map<dovah::form_stub*, idle_node*> idle_forms_to_nodes;
};