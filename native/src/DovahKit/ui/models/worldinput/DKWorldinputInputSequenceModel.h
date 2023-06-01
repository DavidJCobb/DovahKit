#pragma once
#include <optional>
#include <QAbstractItemModel>
#include <QString>
#include "helpers/keyboard/virtual_key.h"
#include "editor/subsystems/worldinput2/input_sequence.h"

class DKWorldinputInputSequenceModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      DKWorldinputInputSequenceModel(QObject* parent = nullptr);
      ~DKWorldinputInputSequenceModel() {
         this->clear();
      }

      using virtual_key   = cobb::keyboard::virtual_key;
      using xinput_button = dovahkit::subsystems::worldinput2::inputs::xinput_button;

      using input_sequence = dovahkit::subsystems::worldinput2::input_sequence;
      //
      using group_type = input_sequence::group_type;

      // For outside actors.
      struct NodeInfo {
         group_type type;
         struct {
            Qt::MouseButton mouse = Qt::MouseButton::NoButton;
            virtual_key     vk = virtual_key::none;
            xinput_button   xinput = xinput_button::none;
         } button;
      };

   protected:
      struct Node {
         ~Node();

         group_type type   = group_type::single_control;
         Node*      parent = nullptr;

         struct {
            Qt::MouseButton mouse  = Qt::MouseButton::NoButton;
            virtual_key     vk     = virtual_key::none;
            xinput_button   xinput = xinput_button::none;
         } button;
         struct {
            QVector<Node*> children; // owned
         } group;

         constexpr bool can_have_children() const noexcept { return this->type != group_type::single_control; }

         void append_child(Node&);
         int  child_count() const noexcept { return this->group.children.size(); }
         int  index_of_child(const Node&) const;
         void insert_child(int i, Node&);
         void remove_child(Node&);

         QString button_name() const;
      };

      Node* _root = nullptr;
      Node* _raycast_associated_button = nullptr;

      bool _is_empty_qmi(const QModelIndex&) const;
      const Node* _node_from_qmi(const QModelIndex&) const;
      Node* _node_from_qmi(const QModelIndex& qmi) {
         return const_cast<Node*>(std::as_const(*this)._node_from_qmi(qmi));
      }
      QModelIndex _qmi_for_node(const Node*, int col = 0) const noexcept;

      void _clear_silent();

   public:
      static constexpr const auto GroupTypeRole           = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr const auto IsRaycastAssociatedRole = (Qt::ItemDataRole)(Qt::UserRole + 2);

      struct Columns {
         Columns() = delete;
         enum : int {
            Name = 0,
            RaycastAssociatedIndicator = 1,
         };
      };
      static constexpr const int MaxColumns = 2;

      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex   index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex   parent(const QModelIndex& index) const;
            virtual QModelIndex   sibling(int row, int column, const QModelIndex& index) const override;
            virtual int           rowCount(const QModelIndex& parent) const override;
            virtual int           columnCount(const QModelIndex& item) const override;

            virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
            virtual QVariant      headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
         #pragma endregion
      #pragma endregion

      void clear();
      void overwriteFromSource(const input_sequence&);
      void overwriteDestination(input_sequence&) const;
      
   protected:
      bool _insertInOrAfter(const QModelIndex& target, Node*);

   public:
      std::optional<QModelIndex> addButtonTo(const QModelIndex& parent);
      std::optional<QModelIndex> addGroupTo(const QModelIndex& parent);
      void deleteItems(QModelIndexList);
      void moveItems(QModelIndexList, int down);

      std::optional<NodeInfo> infoFor(const QModelIndex&) const;
      void replaceInfoFor(const QModelIndex&, const NodeInfo&);

      std::optional<QModelIndex> raycastAssociatedButton() const;
      bool isRaycastAssociatedButton(const QModelIndex&) const;
      bool setRaycastAssociatedButton(const QModelIndex&);
};