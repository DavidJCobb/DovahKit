#pragma once
#include <optional>
#include <QAbstractItemModel>
#include <QString>
#include "helpers/keyboard/virtual_key.h"
#include "editor/subsystems/worldinput2/input_sequence.h"
#include "ui/models/DKGenericTreeModel.h"
#include "widgets/widget-dialogs/DKWorldinputButtonPickDialog.h"

class DKWorldinputInputSequenceModel;

template<>
class DKGenericTreeModelNode<DKWorldinputInputSequenceModel> : public DKGenericTreeModelNodeBase<DKWorldinputInputSequenceModel> {
   public:
      using input_sequence = dovahkit::subsystems::worldinput::input_sequence;
      using group_type = input_sequence::group_type;

      using button_data_type = dovahkit::subsystems::worldinput::inputs::button;

   public:
      group_type type = group_type::concurrent_ordered;
      button_data_type button;

      bool can_have_children() const noexcept {
         return type != group_type::single_control;
      }

      QString button_name() const {
         return DKWorldinputButtonPickDialog::nameOf(this->button);
      }
};

class DKWorldinputInputSequenceModel : public DKGenericTreeModel<DKWorldinputInputSequenceModel> {
   Q_OBJECT;
   friend base_type;
   public:
      struct Columns {
         Columns() = delete;
         enum : int {
            Name = 0,
            RaycastAssociatedIndicator = 1,
         };
      };
      static constexpr const size_t max_columns = 2;

      using DKGenericTreeModel::DKGenericTreeModel; // constructor
      using base_type::clear;
      using base_type::deleteItems;
      using base_type::moveItem;
      using base_type::moveItems;

   public:
      using input_sequence = dovahkit::subsystems::worldinput::input_sequence;
      //
      using group_type = input_sequence::group_type;

      using button_data_type = dovahkit::subsystems::worldinput::inputs::button;

      // For outside actors.
      struct NodeInfo {
         group_type type;
         button_data_type button;
      };

   protected:
      node_type* _raycast_associated_button = nullptr;

   public:
      static constexpr const auto GroupTypeRole           = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const auto IsRaycastAssociatedRole = (Qt::ItemDataRole)(Qt::UserRole + 2);


   #pragma region Stubs, to be overridden on the self type
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

      void overwriteFromSource(const input_sequence&);
      void overwriteDestination(input_sequence&) const;
      
   protected:
      bool _insertInOrAfter(const QModelIndex& target, node_type*);

   public:
      std::optional<QModelIndex> addButtonTo(const QModelIndex& parent);
      std::optional<QModelIndex> addGroupTo(const QModelIndex& parent);

      std::optional<NodeInfo> infoFor(const QModelIndex&) const;
      void replaceInfoFor(const QModelIndex&, const NodeInfo&);

      std::optional<QModelIndex> raycastAssociatedButton() const;
      bool isRaycastAssociatedButton(const QModelIndex&) const;
      bool setRaycastAssociatedButton(const QModelIndex&);
};