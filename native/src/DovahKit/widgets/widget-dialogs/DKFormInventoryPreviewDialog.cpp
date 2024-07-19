#include "./DKFormInventoryPreviewDialog.h"
#include <QBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include "ui/models/DKGenericListModel.h"
#include "widgets/DKHeaderView.h"
#include "dovah/form_stub.h"

// It's a complete waste to even create a model just for this, but QTableWidget 
// is bloody broken and refuses to display anything. Yes, I remembered to heap-
// allocate new QTableWidgetItems. Yes, I used QTableWidgetItem::setData. Yes, 
// I tried using QTableWidget::setItem after setData. Yes, I tried constructing 
// the items with a string argument as a dummy before setting their DisplayRole 
// data.
// 
// QTableWidget is *also* overengineered in an attempt to be generic, and it 
// keeps all of its data private (read: intentionally hidden from your debugger), 
// so it's not actually possible for me to figure out why it isn't doing the one 
// thing it exists to do.
//
// I'm not even going to bother doing things like updating the model nodes when 
// a referred-to form's editor ID changes, at least right now.

struct _ModelNode {
   int     count = 0;
   QString form;
   float   health = 1;
   QString owner;

   _ModelNode() {}
   _ModelNode(const dovah::leveled_list_preview::entry& src) {
      this->count = src.count;
      if (auto* stub = src.form)
         this->form = QString::fromStdString(src.form->editorID);
      if (src.extra.has_value()) {
         auto& src_e = src.extra.value();
         this->health = src_e.health;
         if (auto* stub = src_e.owner)
            this->owner = QString::fromStdString(stub->editorID);
      }
   }
};
class _Model : public DKGenericListModel<_Model, _ModelNode> {
   public:
      static constexpr const size_t column_count = 4;

      using DKGenericListModel::DKGenericListModel;

      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override final {
         if (orientation != Qt::Orientation::Horizontal)
            return {};
         if (role != Qt::DisplayRole && role != Qt::ToolTipRole)
            return {};
         switch (section) {
            case 0: return tr("Count");
            case 1: return tr("Form");
            case 2: return tr("Health");
            case 3: return tr("Owner");
         }
         return {};
      }
      QVariant data_of(const node_type& node, Qt::ItemDataRole role, size_t column) const {
         if (role == Qt::TextAlignmentRole) {
            if (column == 0 || column == 2)
               return (int)(Qt::AlignRight | Qt::AlignVCenter);
            return {};
         }
         if (role == Qt::DisplayRole || role == Qt::ToolTipRole) {
            switch (column) {
               case 0:
                  return node.count;
               case 1:
                  return node.form;
               case 2:
                  return node.health;
               case 3:
                  return node.owner;
            }
         }
         return {};
      }
      Qt::ItemFlags flags_of(const node_type&, size_t column) const {
         return Qt::ItemFlag::ItemIsEnabled | Qt::ItemFlag::ItemIsSelectable | Qt::ItemFlag::ItemNeverHasChildren;
      }

      void overwrite(const std::vector<dovah::leveled_list_preview::entry>& src) {
         this->performReset([this, &src]() {
            this->_nodes.resize(src.size());
            for (size_t i = 0; i < src.size(); ++i)
               this->_nodes[i] = new node_type{ src[i] };
         });
      }
};

DKFormInventoryPreviewDialog::DKFormInventoryPreviewDialog(QWidget* parent) : QDialog(parent) {
   auto* layout = new QVBoxLayout(this);
   this->setLayout(layout);

   this->setWindowTitle(tr("Preview"));

   {
      auto* table = this->subwidgets.table = new QTableView(this);
      layout->addWidget(table);

      table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      table->setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);
      table->setCornerButtonEnabled(false);
      table->setAcceptDrops(false);

      table->setModel(new _Model(table));

      if (auto* vh = table->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
         vh->setVisible(false);
      }

      auto* header = new DKHeaderView(Qt::Horizontal, table);
      header->setFlexResizeEnabled(true);
      table->setHorizontalHeader(header);
      //
      auto metrics = QFontMetrics(table->font());
      header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
      header->setMinimumSectionSize(2);
      header->setColumnFlex(Column::Count,  0, 0, metrics.boundingRect("999").width() * 1.5F + 4);
      header->setColumnFlex(Column::Form,   2, 0);
      header->setColumnFlex(Column::Health, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
      header->setColumnFlex(Column::Owner,  1, 0);
      header->setSectionResizeMode(Column::Count,  QHeaderView::Interactive);
      header->setSectionResizeMode(Column::Form,   QHeaderView::Interactive);
      header->setSectionResizeMode(Column::Health, QHeaderView::Interactive);
      header->setSectionResizeMode(Column::Owner,  QHeaderView::Interactive);
      header->setStretchLastSection(false);
   }

   {
      auto* container    = new QWidget(this);
      auto* inner_layout = new QHBoxLayout(container);
      auto* buttonOK     = new QPushButton(tr("OK"), this);
      layout->addWidget(container);

      container->setLayout(inner_layout);
      inner_layout->addStretch();
      inner_layout->addWidget(buttonOK);
      inner_layout->addStretch();

      QObject::connect(buttonOK, &QPushButton::clicked, this, &QDialog::accept);

      this->setTabOrder(this->subwidgets.table, buttonOK);
   }
}

void DKFormInventoryPreviewDialog::setContents(const std::vector<dovah::leveled_list_preview::entry>& src) {
   auto* table = this->subwidgets.table;

   table->setUpdatesEnabled(false);
   auto* model = (_Model*)table->model();
   model->overwrite(src);
   table->setUpdatesEnabled(true);
}