#include "form_table.h"
#include <QHeaderView>
#include <QLineEdit>
#include "editor/core.h"
#include "./form_table_proxy_model.h"
#include "./form_table_source_model.h"
#include "./object_window_treeview.h"

namespace {
   using Column = ui::object_window::form_table_source_model::Column;
}

FormTable::FormTable(QWidget* parent) : QTableView(parent) {
   auto underlying = new model_type;
   auto proxy      = new proxy_type(this);
   proxy->setSourceModel(underlying);
   this->setModel(proxy);
   this->verticalHeader()->setDefaultSectionSize(0);
   this->sortByColumn(Column::EditorID, Qt::AscendingOrder);
   //
   this->setDragDropMode(QAbstractItemView::DragOnly);
   this->setDragEnabled(true);
   this->setDragDropOverwriteMode(false);
   //
   auto header  = this->horizontalHeader();
   auto metrics = QFontMetrics(this->font());
   header->setDefaultAlignment(Qt::AlignLeft | Qt::AlignBaseline);
   header->setMinimumSectionSize(2);
   header->resizeSection(Column::FormID,    metrics.boundingRect("00000000").width() * 1.5F + 4);
   header->resizeSection(Column::UserCount, metrics.boundingRect("000000").width() * 1.5F + 4);
   header->setSectionResizeMode(Column::EditorID,  QHeaderView::Stretch);
   header->setSectionResizeMode(Column::FormID,    QHeaderView::Interactive);
   header->setSectionResizeMode(Column::UserCount, QHeaderView::Interactive);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete,    this, &FormTable::rebuildModel);
   QObject::connect(&editor, &DovahKitCore::formsRenumberedEnMasse, this, &FormTable::rebuildModel);

   QObject::connect(this->_filterThrottle, &QTimer::timeout, [this]() {
      if (this->_filter)
         this->refilterModel(this->_filter->text());
   });
};
void FormTable::recheckFormTypes() {
   if (!this->_source)
      return;
   auto* model = this->proxyModel();
   assert(model);
   model->setFilterInfo(this->_source->filterInfo());
}
void FormTable::rebuildModel() {
   auto* proxy = this->proxyModel();
   auto* model = this->unwrappedModel();
   assert(model);
   model->rebuild();
}
void FormTable::refilterModel(const QString& text) {
   auto* proxy = this->proxyModel();
   assert(proxy);
   proxy->setFilterFixedString(text);
}
void FormTable::filterChanged() {
   auto& timer = *this->_filterThrottle;
   if (timer.isActive())
      return;
   timer.start(200);
}
void FormTable::filterFinished() {
   this->_filterThrottle->stop();
   if (this->_filter)
      this->refilterModel(this->_filter->text());
}
void FormTable::clear() {
   auto* model = this->unwrappedModel();
   assert(model);
   model->clear();
}
void FormTable::select(dovah::form_stub* stub) {
   auto* proxy = (proxy_type*)this->model();
   assert(proxy);
   auto* model = (model_type*)proxy->sourceModel();
   assert(model);
   auto  index  = model->index(stub);
   auto  mapped = proxy->mapFromSource(index);
   auto* select_model = this->selectionModel();
   if (!select_model)
      return;
   select_model->select(mapped, QItemSelectionModel::ClearAndSelect);
}
void FormTable::setFilter(QLineEdit* field) {
   this->_filterThrottle->stop();
   if (this->_filter) {
      QObject::disconnect(this->_filter, &QLineEdit::textEdited, this, &FormTable::filterChanged);
      QObject::disconnect(this->_filter, &QLineEdit::editingFinished, this, &FormTable::filterFinished);
   }
   this->_filter = field;
   if (!field)
      return;
   this->refilterModel(field->text());
   QObject::connect(field, &QLineEdit::textEdited, this, &FormTable::filterChanged);
   QObject::connect(field, &QLineEdit::editingFinished, this, &FormTable::filterFinished);
}
void FormTable::setSource(ObjectWindowTree* tree) {
   if (this->_source)
      QObject::disconnect(this->_source->selectionModel(), &QItemSelectionModel::selectionChanged , this, &FormTable::recheckFormTypes);
   this->_source = tree;
   if (!tree)
      return;
   this->unwrappedModel()->setBaseFormTypes(tree->allPrimaryFormTypes());
   QObject::connect(tree->selectionModel(), &QItemSelectionModel::selectionChanged, this, &FormTable::recheckFormTypes);
   this->recheckFormTypes();
}

ui::object_window::file_source_requirement FormTable::fileSourceRequirement() const noexcept {
   return this->proxyModel()->fileSourceRequirement();
}
void FormTable::setFileSourceRequirement(ui::object_window::file_source_requirement v) {
   this->proxyModel()->setFileSourceRequirement(v);
}

bool FormTable::onlyShowDeleted() const noexcept {
   return this->proxyModel()->onlyShowDeleted();
}
void FormTable::setOnlyShowDeleted(bool v) {
   this->proxyModel()->setOnlyShowDeleted(v);
}