#include "./DKMagicEffectListWidget.h"
#include <QHeaderView>
#include <QVBoxLayout>
#include "./widget-models/DKMagicEffectListModel.h"

DKMagicEffectListWidget::DKMagicEffectListWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QVBoxLayout(this);
   layout->setMargin(0);
   this->setLayout(layout);

   {
      auto* view = this->_subwidgets.view = new QTableView(this);
      layout->addWidget(view, 1);

      if (auto* vh = view->verticalHeader()) {
         vh->setVisible(false);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      view->setCornerButtonEnabled(false);
      view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      view->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
      view->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
      view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
      view->setWordWrap(false);
   }
   
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->_subwidgets.view);
   #pragma endregion

   #if !defined(QT_PLUGIN)
   {
      auto* widget = this->_subwidgets.view;
      this->_model = new DKMagicEffectListModel(this);
      widget->setModel(this->_model);

      {  // Set up new header
         //
         // Have to do this after setting the model, because QHeaderView::setSectionResizeMode 
         // and friends will crash if the section in question doesn't exist yet.
         //
         auto* header = new QHeaderView(Qt::Orientation::Horizontal, widget);
         widget->setHorizontalHeader(header);

         header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
         header->setMinimumSectionSize(2);
         header->setStretchLastSection(false);
      }
   }
   QObject::connect(this->_subwidgets.view, &QTableView::doubleClicked, this, [this](const QModelIndex& qmi) {
      auto start = qmi.siblingAtColumn(0);
      auto end   = qmi.siblingAtColumn(DKMagicEffectListModel::Column::_COUNT - 1);

      auto* sm = this->_subwidgets.view->selectionModel();
      sm->select(QItemSelection(start, end), QItemSelectionModel::SelectionFlag::ClearAndSelect);

      this->openEditEffectModal();
   });

   static_assert(false, "TODO: Context menu");
   #endif
}

#if !defined(QT_PLUGIN)
   void DKMagicEffectListWidget::setCastingType(const std::optional<dovah::magic_casting_type>& v);

   void DKMagicEffectListWidget::setDeliveryType(const std::optional<dovah::magic_delivery_type>& v);

   void DKMagicEffectListWidget::importFrom(dovah::loaded_forms::Form& owner, const dovah::loaded_forms::components::magic_effect_list& target) {
      this->_model->importFrom(owner, target);
   }
   void DKMagicEffectListWidget::exportTo(dovah::loaded_forms::Form& owner, dovah::loaded_forms::components::magic_effect_list& dst) {
      this->_model->commitTo(owner, dst);
   }
#endif

#if !defined(QT_PLUGIN)
   void DKMagicEffectListWidget::openCreateEffectModal() {
      auto* model = this->_model;
      auto* sm    = this->_subwidgets.view->selectionModel();
      if (!model || !sm)
         return;
      size_t insert_at = std::numeric_limits<size_t>::max();
      auto   rows      = sm->selectedRows();
      if (!rows.isEmpty())
         insert_at = rows.back().row() + 1;
      {
         ui::types::conditions::condition created;
         created.function           = GetIsID;
         created.run_on.type        = ui::types::conditions::run_on_type::subject;
         created.comparison.op      = ui::types::conditions::comparison_operator::equal;
         created.comparison.operand = 1.0F;
         created.reset_parameters();

         auto* modal = new DKMagicEffectListItemDialog(*this->_owning_stub, created, this);
         QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, insert_at]() {
            auto  qmi = model->insertAt(modal->value(), insert_at);
            auto* sm  = this->_subwidgets.view->selectionModel();
            if (!sm)
               return;
            QModelIndex    dummy;
            QModelIndex    br = model->index(qmi.row(), model->columnCount(dummy) - 1, dummy); // (qmi) is just the left "edge" of the selection, and we want to select the whole row
            QItemSelection range(qmi, br);
            sm->select(range, QItemSelectionModel::ClearAndSelect);
         });
         QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
         modal->setWindowModality(Qt::WindowModality::WindowModal);
         modal->open();
      }
   }
   void DKMagicEffectListWidget::openEditEffectModal() {
      auto* model = this->_model;
      auto* sm    = this->_subwidgets.view->selectionModel();
      if (!this->_owning_stub || !model || !sm)
         return;
      auto  rows = sm->selectedRows();
      if (rows.size() != 1)
         return;
      auto  row  = rows[0].row();
      auto* cnd  = model->getCondition(row);
      if (!cnd)
         return;
      {
         auto* modal = new DKMagicEffectListItemDialog(*this->_owning_stub, *cnd, this);
         QObject::connect(modal, &QDialog::accepted, this, [this, model, modal, row, cnd]() {
            const auto after = modal->value();
            model->setCondition(row, after);
         });
         QObject::connect(modal, &QDialog::finished, modal, &QObject::deleteLater);
         modal->setWindowModality(Qt::WindowModality::WindowModal);
         modal->open();
      }
   }
#endif