#pragma once
#include <concepts>
#include <type_traits>
#include <QAbstractItemModel>

//
// This templated class will set up a decent chunk of Qt's model-related boilerplate for you.
//

class QItemSelection;

template<typename Self, typename Node>
class DKGenericListModel : public QAbstractItemModel {
   public:
      using base_type = DKGenericListModel;
      using self_type = Self;
      using node_type = Node;

      static constexpr const size_t column_count = 1; // you can override this

   protected:
      QVector<node_type*> _nodes;

   public:
      DKGenericListModel(QObject* parent) : QAbstractItemModel(parent) {}
      ~DKGenericListModel();

   private:
      constexpr bool qmi_is_mine(const QModelIndex& qmi) const noexcept;
      constexpr bool qmi_is_none(const QModelIndex& qmi) const noexcept;

      const node_type* node_for_qmi(const QModelIndex& qmi) const;
      node_type* node_for_qmi(const QModelIndex& qmi) { return const_cast<node_type*>(std::as_const(*this).node_for_qmi(qmi)); }
      QModelIndex qmi_for_node(const node_type&, size_t column = 0) const;

   #pragma region Stubs, to be overridden on the self type
   protected:
      void on_before_delete_node(node_type&) {} // called before deleting any node

      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;

      // Controls whether the user can drop items into the view generally (as opposed to 
      // onto a specific item, overwriting that item).
      bool allow_inbound_drag_and_drop() const {
         return false;
      }
   #pragma endregion

   private:
      void _clear_silent();

   public:
      #pragma region QAbstractItemModel overrides
         #pragma region Hierarchy
            virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override;
            virtual QModelIndex parent(const QModelIndex& index) const;
            virtual QModelIndex sibling(int row, int column, const QModelIndex& index) const override;
            virtual int         rowCount(const QModelIndex& parent) const override final;
            virtual int         columnCount(const QModelIndex& item) const override final;

            virtual bool moveRows(const QModelIndex& from_parent, int first_row_index, int count, const QModelIndex& to_parent, int to_position) override;
         #pragma endregion
         #pragma region Node data
            virtual QVariant      data(const QModelIndex& index, int role) const override;
            virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
         #pragma endregion
      #pragma endregion

      int rowCount() const;

   // Subclasses can expose these if they wish:
   protected:
      void clear();
      bool deleteItems(int at, int count);
      void moveItem(const QModelIndex&, int down);
      void moveItems(const QModelIndex& start, const QModelIndex& end, int down);
      void moveItems(const QItemSelection&, int down);

   #pragma region Subclass helpers
   protected:
      const node_type* node(const QModelIndex&) const;
      node_type* node(const QModelIndex& qmi) {
         return const_cast<node_type*>(std::as_const(*this).node(qmi));
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

      template<typename Functor>
      void performReset(Functor&& f) {
         this->beginResetModel();
         this->_clear_silent();
         f();
         this->endResetModel();
      }
   #pragma endregion
};

#include "./DKGenericListModel.inl"