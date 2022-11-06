#pragma once
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include "editor/subsystems/worldinput/bind_tree/tree.h"

class Options3DControlSchemeModel : public QAbstractItemModel {
   Q_OBJECT;
   friend class Options3DControlSchemeTreeModel;
   public:
      static constexpr auto BoundInputRole    = (Qt::ItemDataRole)(Qt::UserRole + 0);
      static constexpr auto InputNodeNameRole = (Qt::ItemDataRole)(Qt::UserRole + 1);
      static constexpr auto InputNodeToolRole = (Qt::ItemDataRole)(Qt::UserRole + 2);
      static constexpr auto NodeTypeRole      = (Qt::ItemDataRole)(Qt::UserRole + 3);
   public:
      Options3DControlSchemeModel(QObject* parent = nullptr);
      ~Options3DControlSchemeModel();

      dovahkit::subsystems::worldinput::input_device_type inputDevice() const { return this->_state.tree.device; }
      const dovahkit::subsystems::worldinput::binds::tree& tree() const { return this->_state.tree; }

   public slots:
      void setTree(const dovahkit::subsystems::worldinput::binds::tree&);

   protected:
      struct {
         dovahkit::subsystems::worldinput::binds::tree tree = dovahkit::subsystems::worldinput::binds::tree(dovahkit::subsystems::worldinput::input_device_type::keyboard_mouse);
      } _state;

      bool is_real_root(const QModelIndex&) const;
      QModelIndex qmi_from_real_root(int col = 0) const { return this->createIndex(0, col, (void*)this); }
      QModelIndex qmi_from_node(const dovahkit::subsystems::worldinput::binds::node*, int col = 0) const;
      dovahkit::subsystems::worldinput::binds::node* node_from_qmi(const QModelIndex&) const;

   public:
      virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
      virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;
      virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
      virtual bool hasChildren(const QModelIndex& parent = QModelIndex()) const override;
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
      virtual QModelIndex parent(const QModelIndex& index) const override;
      virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
      virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

      virtual bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) override;
      virtual bool insertRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
      virtual bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;

      void nonCursedMoveRow(const QModelIndex& source_parent, int row, const QModelIndex& dest_parent, int down);

      inline dovahkit::subsystems::worldinput::binds::node* node(const QModelIndex& qmi) const {
         return node_from_qmi(qmi);
      }
      void setNodeEdited(const dovahkit::subsystems::worldinput::binds::node* node) {
         auto start = this->qmi_from_node(node, 0);
         auto end   = this->qmi_from_node(node, this->columnCount() - 1);
         emit this->dataChanged(start, end);
      }
};

class Options3DControlSchemeTreeModel : public QSortFilterProxyModel {
   Q_OBJECT;
   public:
      virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;
};