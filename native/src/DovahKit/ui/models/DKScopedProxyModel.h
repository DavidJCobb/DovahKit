#pragma once
#include <vector>
#include <QAbstractItemModel>
#include <QIdentityProxyModel>

class DKScopedProxyModel : public QIdentityProxyModel {
   Q_OBJECT;
   protected:
      struct qmi_mapping {
         QModelIndex source;
         QModelIndex proxy;
      };
      using pending_qpmi_remap_list = std::vector<qmi_mapping>;

   public:
      using QIdentityProxyModel::QIdentityProxyModel;

      std::optional<QModelIndex> rootIndex() const;
      void setRootIndex(const std::optional<QModelIndex>& source_qmi);

      // Control whether the item specified by the `rootIndex` is visible in the 
      // tree. Use this if you want to scope the model to a given QMI, but also 
      // have that QMI itself visible as the only top-level item in the proxy 
      // model. This has no effect when the proxy is scoped to the source model 
      // root.
      //
      // If the source-model root is visible, then the proxy-model's "real" root, 
      // represented by an invalid QModelIndex, is referred to as the "super-root."
      bool isRootVisible() const;
      void setRootVisible(bool);

      #pragma region QAbstractItemModel overrides
         virtual Qt::ItemFlags flags(const QModelIndex&) const override;
         virtual QModelIndex index(int row, int column, const QModelIndex& proxy_parent_qmi) const override;
         virtual int columnCount(const QModelIndex&) const override;
         virtual int rowCount(const QModelIndex&) const override;
         virtual QModelIndex parent(const QModelIndex&) const override;
         virtual QModelIndex sibling(int row, int col, const QModelIndex&) const override;
         #pragma region Modify hierarchy
            virtual bool dropMimeData(const QMimeData*, Qt::DropAction, int row, int column, const QModelIndex& parent) override;
            virtual bool insertColumns(int column, int count, const QModelIndex&) override;
            virtual bool insertRows(int column, int count, const QModelIndex&) override;
            virtual bool moveColumns(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) override;
            virtual bool moveRows(const QModelIndex& from_parent, int first, int last, const QModelIndex& to_parent, int to) override;
            virtual bool removeColumns(int column, int count, const QModelIndex&) override;
            virtual bool removeRows(int column, int count, const QModelIndex&) override;
         #pragma endregion
      #pragma endregion
      #pragma region QAbstractProxyModel overrides
         virtual QModelIndex mapFromSource(const QModelIndex& source_qmi) const override;
         virtual QModelIndex mapToSource(const QModelIndex& proxy_qmi) const override;
         virtual void setSourceModel(QAbstractItemModel* sourceModel) override;
      #pragma endregion

   protected:
      std::optional<QPersistentModelIndex> _root_index;
      bool  _root_is_visible = false;
      std::vector<pending_qpmi_remap_list> _pending_layout_changes;

   protected:
      bool _col_range_includes_root(const QModelIndex& source_parent_qmi, int first, int last) const;
      bool _row_range_includes_root(const QModelIndex& source_parent_qmi, int first, int last) const;

      bool _root_is_or_contains(const QModelIndex& source_qmi) const;
      bool _root_contains(const QModelIndex& source_qmi) const;

      bool _proxy_qmi_is_root(const QModelIndex& proxy_qmi) const;
      bool _proxy_qmi_is_super_root(const QModelIndex&) const;

      // Use when you already know `source_qmi` is the root index or is inside of the root index.
      QModelIndex _unchecked_map_from_source(const QModelIndex& source_qmi) const;

      QModelIndex _child_qmi_of_visible_root(int col = 0) const;
      bool _is_parent_of_visible_root(const QModelIndex& proxy_qmi) const;
};
