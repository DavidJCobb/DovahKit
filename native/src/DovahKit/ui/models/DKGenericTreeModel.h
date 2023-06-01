#pragma once
#include <concepts>
#include <type_traits>
#include <QAbstractItemModel>

//
//  - Specialize DKGenericTreeModelNode.
// 
//  - Define a subclass of DKGenericTreeModel using CRTP.
// 
// This templated class will set up a decent chunk of Qt's model-related boilerplate for you.
//

class QItemSelection;

template<typename Self>
class DKGenericTreeModel;

// Specialize this, and inherit from DKGenericTreeModelNodeBase<Model>.
template<class Model>
class DKGenericTreeModelNode;

template<class Model>
class DKGenericTreeModelNodeBase {
   template<typename Self> friend class DKGenericTreeModel;
   public:
      using model_type = Model;
      using node_type  = DKGenericTreeModelNode<Model>;

      ~DKGenericTreeModelNodeBase();

   protected:
      QVector<node_type*> _children;
      node_type* _parent = nullptr;

      bool _check_can_have_children() const noexcept {
         return ((const node_type*)this)->can_have_children();
      }

   public:
      constexpr const QVector<const node_type*>& children() const noexcept {
         return reinterpret_cast<const QVector<const node_type*>&>(this->_children);
      }

      size_t child_count() const noexcept;
      bool contains(const node_type&) const noexcept;
      size_t index_of_child(const node_type&) const noexcept;

      const node_type* nth_child(size_t) const noexcept;
      node_type* nth_child(size_t n) noexcept { return const_cast<node_type*>(std::as_const(*this).nth_child(n)); }

      const node_type* parent_node() const noexcept;
      node_type* parent_node() noexcept { return const_cast<node_type*>(std::as_const(*this).parent_node()); }

      void move_children(size_t start, size_t count, size_t destination_start);
      void move_children_to(size_t start, size_t count, node_type& destination_parent, size_t destination_start);

      void append_child(node_type&);
      void insert_child(size_t at, node_type&);
      void remove_child(node_type&);

      // Subclass can override this. A default-constructed node must have whatever data 
      // would cause this to return `true`.
      bool can_have_children() const noexcept {
         return true;
      }
};

template<typename Self>
class DKGenericTreeModel : public QAbstractItemModel {
   public:
      using base_type = DKGenericTreeModel;
      using self_type = Self;
      using node_type = DKGenericTreeModelNode<Self>;

      static constexpr const size_t max_columns = 1; // you can override this
      static constexpr const bool variable_column_count = false; // you can override this

   protected:
      node_type invisible_root;

   public:
      DKGenericTreeModel(QObject* parent) : QAbstractItemModel(parent) {}
      ~DKGenericTreeModel();

   private:
      constexpr bool qmi_is_mine(const QModelIndex& qmi) const noexcept;
      constexpr bool qmi_is_none(const QModelIndex& qmi) const noexcept;

      const node_type* node_for_qmi(const QModelIndex& qmi) const;
      node_type* node_for_qmi(const QModelIndex& qmi) { return const_cast<node_type*>(std::as_const(*this).node_for_qmi(qmi)); }
      QModelIndex qmi_for_node(const node_type&, size_t column = 0) const;

   #pragma region Stubs, to be overridden on the self type
   protected:
      size_t column_count_of(const node_type& node) const requires(variable_column_count);
      void on_before_delete_node(node_type&) {} // called before deleting any node

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
   #pragma endregion

   private:
   void _clear_silent();

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

   #pragma region Subclass helpers
   protected:
      const node_type* node(const QModelIndex&) const;
      node_type* node(const QModelIndex& qmi) {
         return const_cast<node_type*>(std::as_const(*this).node(qmi));
      }
      
      const node_type* parentNode(const QModelIndex&) const;
      node_type* parentNode(const QModelIndex& qmi) {
         return const_cast<node_type*>(std::as_const(*this).parentNode(qmi));
      }

      QModelIndex index(const node_type*, size_t column = 0) const;

      static constexpr bool isSameRow(const QModelIndex& a, const QModelIndex& b) noexcept {
         if (a.row() != b.row())
            return false;
         if (a.model() != b.model())
            return false;
         if (a.internalPointer() != b.internalPointer())
            return false;
         return true;
      }

      void emitNodeChanged(const node_type&);
      void emitNodeChanged(const node_type&, size_t column);
      void emitNodeChanged(const QModelIndex&);

      void replaceAll(node_type* root);
   #pragma endregion
};

#include "./DKGenericTreeModel.inl"