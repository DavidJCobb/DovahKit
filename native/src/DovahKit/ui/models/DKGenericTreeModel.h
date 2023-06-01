#pragma once
#include <concepts>
#include <type_traits>
#include <QAbstractItemModel>

class QItemSelection;

template<typename Node>
concept DKGenericTreeModelNode = requires(Node& n, const Node& cn, size_t i) {
   { cn.child_count() } -> std::same_as<size_t>;
   { cn.contains(cn) } -> std::same_as<bool>; // is the argument a child or descendant of the context?
   { cn.index_of_child(cn) } -> std::same_as<size_t>; // return index of argument in context's child list, or -1 if argument is not a child

   {  n.nth_child(i) } -> std::same_as<Node*>;
   { cn.nth_child(i) } -> std::same_as<const Node*>;
   {  n.nth_child(i) } -> std::same_as<Node*>;
   { cn.parent_node() } -> std::same_as<const Node*>;
   {  n.parent_node() } -> std::same_as<Node*>;

   { n.move_children(i, i, i) }; // args: start_index, count, destination start index (element before which we want to place the first moved child)
   //
   // Moves a group of children within the node. Easiest way to accomplish this assuming a QVector is:
   // 
   //    while (count--)
   //       this->children.move(start_index + count, destination_start_index);
   //

   { n.move_children_to(i, i, n, i) }; // args: start index, count, destination node, destination start index

   // Additional requirements:
   //  - Deleting a node should automatically remove it from its parent node.
};

template<typename Self, DKGenericTreeModelNode Node>
class DKGenericTreeModel : public QAbstractItemModel {
   public:
      using self_type = Self;
      using node_type = Node;

      static constexpr const size_t max_columns = 1; // you can override this
      static constexpr const bool variable_column_count = false; // you can override this

   protected:
      node_type* _root = nullptr;

   public:
      DKGenericTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

   private:
      constexpr bool qmi_is_mine(const QModelIndex& qmi) const noexcept;
      constexpr bool qmi_is_none(const QModelIndex& qmi) const noexcept;
      constexpr bool qmi_is_root(const QModelIndex& qmi) const noexcept;

      constexpr QModelIndex make_qmi_for_root(size_t column = 0) const noexcept;

      const node_type* parent_node_for_qmi(const QModelIndex& qmi) const;
      node_type* parent_node_for_qmi(const QModelIndex& qmi);
      const node_type* node_for_qmi(const QModelIndex& qmi) const;
      node_type* node_for_qmi(const QModelIndex& qmi);
      QModelIndex qmi_for_node(const node_type&, size_t column = 0) const;

   #pragma region Stubs, to be overridden on the self type
   protected:
      size_t column_count_of(const node_type& node) const requires(variable_column_count);
      void on_before_delete_root() {}

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
   #pragma endregion

   private:
      void _clear_silent() {
         if (this->_root) {
            this->_on_before_delete_root();
            delete this->_root;
            this->_root = nullptr;
         }
      }

   public:
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
         #pragma endregion
      #pragma endregion

   // Subclasses can expose these if they wish:
   protected:
      void clear();
      void deleteItems(QModelIndexList);
      void moveItem(const QModelIndex&, int down);
      void moveItems(const QModelIndex& start, const QModelIndex& end, int down);
      void moveItems(const QItemSelection&, int down);

      const node_type* node(const QModelIndex&) const;
      node_type* node(const QModelIndex& qmi) {
         return const_cast<node_type*>(std::as_const(*this).node(qmi));
      }

      QModelIndex index(const node_type*) const;
};

#include "./DKGenericTreeModel.inl"