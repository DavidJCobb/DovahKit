#include "DKBSABrowseDialog.h"
#include <QApplication>
#include <QBoxLayout>
#include <QPushButton>
#include "../widget-models/DKBSACollectionModel.h"

DKBSABrowseDialog::DKBSABrowseDialog(QWidget* parent) : QDialog(parent) {
   auto* style = QApplication::style();
   //
   auto* path = this->subwidgets.path     = new QLineEdit(this);
   auto* view = this->subwidgets.view     = new QListView(this);
   auto* name = this->subwidgets.filename = new QLineEdit(this);
   {
      auto* layout = new QVBoxLayout(this);
      {
         static_assert(false, "TODO: Revision needed: use a QToolbar to hold both upOneLevel and path");
         static_assert(false, "TODO: Consider adding a search dialog matching the Win10 file picker");
         //
         auto* up = this->subwidgets.upOneLevel = new QToolButton(this);
         up->setIcon(style->standardIcon(QStyle::SP_FileDialogToParent));
         QObject::connect(up, &QToolButton::clicked, this, [this]() {
            auto* view  = this->subwidgets.view;
            auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
            if (!model)
               return;
            auto index = view->rootIndex();
            if (!index.isValid())
               return;
            if (index == this->state.pathStemIndex) // don't allow navigating up above the stem
               return;
            view->setRootIndex(model->parent(index));
         });
         //
         auto* nested = new QHBoxLayout(this);
         nested->addWidget(path);
         layout->addLayout(nested);
      }
      layout->addWidget(view);
      layout->addWidget(name);
      layout->setStretch(0, 0);
      layout->setStretch(1, 1);
      layout->setStretch(0, 0);
      //
      auto* nested = new QHBoxLayout(this);
      static_assert(false, "TODO: Add a button somewhere to let users pick a loose file instead");
      auto* ok     = new QPushButton(tr("Open"), this);
      auto* cancel = new QPushButton(tr("Cancel"), this);
      nested->addStretch(0);
      nested->addWidget(ok);
      nested->addWidget(cancel);
      layout->addLayout(nested, 0);
      QObject::connect(ok, &QPushButton::clicked, this, [this]() {
         auto* view  = this->subwidgets.view;
         auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
         auto* sm    = view->selectionModel();
         if (!model || !sm)
            return;
         auto index = sm->currentIndex();
         if (model->isFolder(index)) {
            view->setRootIndex(index);
         } else if (model->isFile(index)) {
            auto data = model->data(index, DKBSACollectionModel::FullPathRole);
            if (data.isValid() && data.type() == QMetaType::QString) {
               emit this->fileSelected(data.toString());
               this->accept();
            }
            this->reject();
         }
      });
      QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
   }
   //
   auto* model = new DKBSACollectionModel(this);
   view->setModel(model);
   //
   QObject::connect(path, &QLineEdit::returnPressed, this, [this]() {
      static_assert(false, "TODO: Hitting Enter in the path box should change the targeted path (allow './' and '../', and either way, make sure to validate against the pathStem!)");
   });
   QObject::connect(name, &QLineEdit::returnPressed, this, [this]() {
      static_assert(false, "TODO: Hitting Enter in the filename box should select the named file, if there is one, or enter the named folder, if there is one.");
   });
}

void DKBSABrowseDialog::setPathStem(const QString& stem) {
   if (stem == this->state.pathStem)
      return;
   static_assert(false, "TODO: normalize the path (no './' or '../')! consider having the model's indexOfFolder function do so as well!");
   this->state.pathStem = stem;
   //
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (model) {
      this->state.pathStemIndex = model->indexOfFolder(stem);
      view->setRootIndex(this->state.pathStemIndex);
   } else {
      this->state.pathStemIndex = QModelIndex();
   }
}