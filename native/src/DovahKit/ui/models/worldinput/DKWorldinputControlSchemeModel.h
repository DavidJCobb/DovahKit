#pragma once
#include <optional>
#include <variant>
#include <QAbstractItemModel>
#include <QString>
#include "helpers/tree/data_variant.h"
#include "helpers/keyboard/virtual_key.h"
#include "editor/subsystems/worldinput/control_scheme/all_node_headers.h"
#include "editor/subsystems/worldinput/control_scheme.h"
#include "ui/models/DKGenericTreeModel.h"

class DKWorldinputControlSchemeModel;

template<>
class DKGenericTreeModelNode<DKWorldinputControlSchemeModel> : public DKGenericTreeModelNodeBase<DKWorldinputControlSchemeModel> {
   public:
      using source_node_type = dovahkit::subsystems::worldinput::control_scheme::node;
      using variant_type     = cobb::node_data_variant<source_node_type, true>;

      variant_type data;
      struct {
         QString name;
         QString bound_tool_name;
         QString input_sequence;
      } cached;

      constexpr bool can_have_children() const noexcept {
         if (std::holds_alternative<std::monostate>(this->data))
            return true;
         return !std::holds_alternative<dovahkit::subsystems::worldinput::control_scheme_action>(this->data);
      }
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
      using control_scheme_type = dovahkit::subsystems::worldinput::control_scheme;

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

   public:

      // returns QMI of inserted node, if node is inserted
      QModelIndex insertAfter(const QModelIndex& after, const node_type::variant_type&);

      node_type::variant_type infoFor(const QModelIndex&) const;
      void replaceInfoFor(const QModelIndex&, const node_type::variant_type&);

      // Attempts to add the provided node as a child of the specified parent. If the specified 
      // parent is a leaf node, we add the provided node as a next-sibling if possible.
      //
      // If we successfully add the provided node, then we also take ownership of it. If we fail 
      // to add it to the model, then we delete it outright.
      //std::optional<QModelIndex> addNodeTo(node_type::underlying_type*, const QModelIndex& parent);
};