#include "./DKLeveledListPreviewDialog.h"
#include <QBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include "../DKHeaderView.h"
#include "../widget-models/DKLeveledListPreviewModel.h"

DKLeveledListPreviewDialog::DKLeveledListPreviewDialog(QWidget* parent) : QDialog(parent) {
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

      table->setModel(new DKLeveledListPreviewModel(table));

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
      header->setColumnFlex(DKLeveledListPreviewModel::Column::Count,  0, 0, metrics.boundingRect("9999").width() * 1.5F + 4);
      header->setColumnFlex(DKLeveledListPreviewModel::Column::Form,   2, 0);
      header->setColumnFlex(DKLeveledListPreviewModel::Column::Health, 0, 0, metrics.boundingRect("100%").width() * 1.5F + 4);
      header->setColumnFlex(DKLeveledListPreviewModel::Column::Owner,  1, 0);
      header->setSectionResizeMode(DKLeveledListPreviewModel::Column::Count,  QHeaderView::Interactive);
      header->setSectionResizeMode(DKLeveledListPreviewModel::Column::Form,   QHeaderView::Interactive);
      header->setSectionResizeMode(DKLeveledListPreviewModel::Column::Health, QHeaderView::Interactive);
      header->setSectionResizeMode(DKLeveledListPreviewModel::Column::Owner,  QHeaderView::Interactive);
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

void DKLeveledListPreviewDialog::setContents(const std::vector<dovah::leveled_list_preview::entry>& src) {
   auto* table = this->subwidgets.table;

   table->setUpdatesEnabled(false);
   auto* model = (DKLeveledListPreviewModel*)table->model();
   model->overwrite(src);
   table->setUpdatesEnabled(true);
}