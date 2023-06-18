#pragma once
#include <optional>
#include <QAbstractItemModel>
#include <QString>
#include "helpers/keyboard/virtual_key.h"
#include "editor/subsystems/worldinput2/bind_tree/node.h"
#include "editor/subsystems/worldinput2/bind_tree/tree.h"
#include "ui/models/DKGenericTreeModel.h"

class DKWorldinputControlSchemeModel;

template<>
class DKGenericTreeModelNode<DKWorldinputControlSchemeModel> : public DKGenericTreeModelNodeBase<DKWorldinputControlSchemeModel> {
   public:
      using underlying_type = dovahkit::subsystems::worldinput2::binds::node;

      underlying_type* underlying_data = nullptr;
      struct {
         QString name;
         QString bound_tool_name;
         QString input_sequence;
      } cached;
};

class DKWorldinputControlSchemeModel : public DKGenericTreeModel<DKWorldinputControlSchemeModel> {
   Q_OBJECT;
   friend base_type;
   public:
      struct Columns {
         Columns() = delete;
         enum : int {
            Name     = 0,
            Behavior = 1, // "Modifier", "Editor mode check", or bound tool name
            Value    = 2, // input sequence (modifiers; tools) or editor mode
         };
      };
      static constexpr const size_t max_columns = 3;

      using DKGenericTreeModel::DKGenericTreeModel; // constructor
      using base_type::clear;
      using base_type::deleteItems;
      using base_type::moveItem;
      using base_type::moveItems;

   public:
      using control_scheme_type = dovahkit::subsystems::worldinput2::binds::tree;

   public:
      static constexpr const auto NodeTypeRole = (Qt::ItemDataRole)(Qt::UserRole + 1);

   #pragma region Overrides
   protected:
      void on_before_delete_node(node_type& n);
      void on_before_delete_root();

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
   #pragma endregion

   protected:
      const node_type* root_node() const noexcept;
      node_type* root_node() noexcept { return const_cast<node_type*>(std::as_const(*this).root_node()); };

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Node data
            virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
         #pragma endregion
      #pragma endregion

      void overwriteFromSource(const control_scheme_type&);
      void overwriteDestination(control_scheme_type&) const;
      
   protected:
      bool _insertInOrAfter(const QModelIndex& target, node_type*);

   public:
      std::optional<QModelIndex> addBindNodeTo(const QModelIndex& parent);
      std::optional<QModelIndex> addModifierNodeTo(const QModelIndex& parent);
      std::optional<QModelIndex> addEditorModeNodeTo(const QModelIndex& parent);

      //std::optional<NodeInfo> infoFor(const QModelIndex&) const;
      //void replaceInfoFor(const QModelIndex&, const NodeInfo&);
};