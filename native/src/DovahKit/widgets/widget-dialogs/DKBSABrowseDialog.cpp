#include "DKBSABrowseDialog.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include "../widget-models/DKBSACollectionModel.h"
#include "../../editor/core.h"

DKBSABrowseDialog::DKBSABrowseDialog(QWidget* parent) : QDialog(parent) {
   auto* style = QApplication::style();
   {
      std::filesystem::path path;
      //
      auto& editor = DovahKitCore::get();
      editor.get_game_path(path, editor.get_current_game());
      //
      this->state._looseFilePath = QDir::cleanPath(QString::fromUtf8((const char*)path.u8string().c_str()) + "/Data");
      if (QDir(this->state._looseFilePath).exists()) {
         this->state._looseFilePath.clear();
      }
   }
   //
   auto* path = this->subwidgets.path     = new QLineEdit(this);
   auto* view = this->subwidgets.view     = new QListView(this);
   auto* name = this->subwidgets.filename = new QLineEdit(this);
   {
      auto* layout = new QVBoxLayout(this);
      {
         auto* toolbar = new QToolBar(this);
         layout->addWidget(toolbar);
         //
         auto* up = this->subwidgets.upOneLevel = new QToolButton(this);
         up->setIcon(style->standardIcon(QStyle::SP_FileDialogToParent));
         QObject::connect(up, &QToolButton::clicked, this, &DKBSABrowseDialog::upOneLevel);
         //
         {
            auto sp = path->sizePolicy();
            sp.setHorizontalStretch(1);
            path->setSizePolicy(sp);
         }
         //
         auto* vc = this->subwidgets.viewMode = new QToolButton(this);
         {
            auto* menu = new QMenu(vc);
            auto* action_list = new QAction(tr("List view"), menu);
            auto* action_grid = new QAction(tr("Icon view"), menu);
            action_list->setIcon(style->standardIcon(QStyle::SP_FileDialogListView));
            action_grid->setIcon(style->standardIcon(QStyle::SP_FileDialogContentsView));
            QObject::connect(action_list, &QAction::triggered, this, [this]() { this->setViewMode(QListView::ViewMode::ListMode); });
            QObject::connect(action_grid, &QAction::triggered, this, [this]() { this->setViewMode(QListView::ViewMode::IconMode); });
            menu->addAction(action_list);
            menu->addAction(action_grid);
            vc->setMenu(menu);
            //
            vc->setIcon(style->standardIcon(QStyle::SP_FileDialogListView));
            //
            vc->setPopupMode(QToolButton::ToolButtonPopupMode::MenuButtonPopup);
         }
         //
         toolbar->addWidget(up);
         toolbar->addWidget(path);
         toolbar->addWidget(vc);
      }
      layout->addWidget(view);
      layout->addWidget(name);
      layout->setStretch(0, 0);
      layout->setStretch(1, 1);
      layout->setStretch(0, 0);
      //
      auto* nested = new QHBoxLayout(this);
      auto* loose  = new QPushButton(tr("Loose file..."), this);
      auto* ok     = new QPushButton(tr("Open"), this);
      auto* cancel = new QPushButton(tr("Cancel"), this);
      nested->addStretch(0);
      nested->addWidget(loose);
      nested->addWidget(ok);
      nested->addWidget(cancel);
      layout->addLayout(nested, 0);
      if (this->state._looseFilePath.isEmpty()) {
         loose->setEnabled(false);
      } else {
         QObject::connect(loose, &QPushButton::clicked, this, &DKBSABrowseDialog::offerLooseFile);
      }
      QObject::connect(ok,     &QPushButton::clicked, this, &DKBSABrowseDialog::openSelectedNode);
      QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
   }
   //
   auto* model = new DKBSACollectionModel(this);
   view->setUniformItemSizes(true);
   view->setBatchSize(200);
   view->setLayoutMode(QListView::LayoutMode::Batched);
   view->setModel(model);
   QObject::connect(model, &QAbstractItemModel::modelReset, this, [this, view, model]() {
      if (this->state.pathStem.isEmpty())
         return;
      this->state.pathStemIndex = model->indexOfFolder(this->state.pathStem);
      view->setRootIndex(this->state.pathStemIndex);
   });
   QObject::connect(view, &QAbstractItemView::doubleClicked, this, [this](const QModelIndex& target) {
      if (!target.isValid())
         return;
      this->openNode(target);
   });
   //
   QObject::connect(path, &QLineEdit::returnPressed, this, [this, path]() {
      this->selectPath(path->text());
   });
   QObject::connect(name, &QLineEdit::returnPressed, this, [this, name]() {
      this->selectFileByName(name->text());
   });
   //
   //
   //
   QObject::connect(this, &DKBSABrowseDialog::directoryEntered, this, [this](const QString& path) {
      this->subwidgets.path->setText(path);
      //
      if (auto* sm = this->subwidgets.view->selectionModel()) {
         sm->clear(); // this doesn't occur automatically when changing the root index, unfortunately
      }
      this->subwidgets.filename->setText(QString());
   });
   if (auto* sm = view->selectionModel()) {
      QObject::connect(sm, &QItemSelectionModel::selectionChanged, this, &DKBSABrowseDialog::_updateFilenameTextFromSelection);
   }
}

QString DKBSABrowseDialog::directory() const noexcept {
   auto* view = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return QString();
   auto qmi = view->rootIndex();
   if (qmi.isValid())
      return model->data(view->rootIndex(), DKBSACollectionModel::FullPathRole).toString();
   return QString();
}

/*static*/ QString DKBSABrowseDialog::getOpenFileName(
   QWidget* parent,
   const QString& caption,
   const QString& pathStem,
   const QString& initial,
   const QString& filter,
   QString* selectedFilter,
   DKBSACollectionModelBackend* backend
) {
   auto* dialog = new DKBSABrowseDialog(parent);
   dialog->setWindowTitle(caption);
   dialog->setPathStem(pathStem);
   if (backend)
      dialog->setBackend(backend);
   if (!initial.isEmpty())
      dialog->setDirectoryAndFile(initial);
   dialog->setWindowModality(Qt::WindowModality::WindowModal);
   dialog->exec();
   auto result = dialog->state._finalResult;
   delete dialog;
   return result;
}

void DKBSABrowseDialog::acceptWithFile(const QString& path) {
   this->state._finalResult = path;
   emit this->fileSelected(path);
   this->accept();
}
void DKBSABrowseDialog::offerLooseFile() {
   QDir minimum = this->state._looseFilePath;
   if (minimum.exists(this->state.pathStem))
      minimum.cd(this->state.pathStem);
   //
   auto dialog = new QFileDialog(this);
   dialog->setOptions(QFileDialog::Option::DontResolveSymlinks | QFileDialog::Option::ReadOnly);
   dialog->setDirectory(minimum);
   dialog->setWindowModality(Qt::WindowModality::WindowModal);
   dialog->setWindowTitle(tr("Select loose file..."));
   QObject::connect(dialog, &QFileDialog::directoryEntered, this, [dialog, minimum](const QString& entered) {
      if (minimum.relativeFilePath(entered).startsWith("../")) {
         dialog->setDirectory(minimum);
         QApplication::beep();
      }
   });
   QObject::connect(dialog, &QFileDialog::fileSelected, this, [this, minimum](const QString& path) {
      if (path.isEmpty())
         return;
      auto rel = minimum.relativeFilePath(path);
      if (rel.startsWith("../"))
         return;
      this->acceptWithFile(this->state.pathStem + '/' + rel);
   });
   QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
   dialog->show();
}
void DKBSABrowseDialog::openNode(const QModelIndex& index) {
   if (!index.isValid())
      return;
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return;
   if (model->isFolder(index)) {
      view->setRootIndex(index);
      emit this->directoryEntered(model->fullPathTo(index));
   } else if (model->isFile(index)) {
      auto data = model->data(index, DKBSACollectionModel::FullPathRole);
      if (data.isValid() && data.type() == QMetaType::QString) {
         this->acceptWithFile(data.toString());
         return;
      }
      this->reject();
   }
}
void DKBSABrowseDialog::openSelectedNode() {
   auto* view  = this->subwidgets.view;
   auto* sm    = view->selectionModel();
   if (!sm)
      return;
   auto index = sm->currentIndex();
   this->openNode(index);
}
void DKBSABrowseDialog::selectFileByName(QString name) {
   if (name.isEmpty() || name.contains('/') || name.contains('\\'))
      return;
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return;
   QModelIndex index = view->rootIndex();
   QModelIndex file  = model->indexOfFile(name, index);
   if (!file.isValid())
      return;
   auto data = model->data(index, DKBSACollectionModel::FullPathRole);
   if (data.isValid() && data.type() == QMetaType::QString) {
      this->acceptWithFile(data.toString());
      return;
   }
   this->reject();
}
void DKBSABrowseDialog::selectPath(const QString& path) {
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return;
   auto index   = view->rootIndex();
   auto cleaned = QDir::cleanPath(path);
   index = model->indexOfFolder(cleaned, index);
   if (!index.isValid()) {
      index = model->indexOfFolder(cleaned); // try treating the path as absolute
      if (!index.isValid())
         return;
   }
   view->setRootIndex(index);
   emit this->directoryEntered(cleaned);
}
void DKBSABrowseDialog::setViewMode(QListView::ViewMode vm) {
   auto* view = this->subwidgets.view;
   if (vm == view->viewMode())
      return;
   view->setUpdatesEnabled(false);
   //
   auto* style  = QApplication::style();
   auto* button = this->subwidgets.viewMode;
   //
   view->setViewMode(vm);
   switch (vm) {
      using _ = decltype(vm);
      case _::ListMode:
         view->setUniformItemSizes(true);
         view->setBatchSize(200);
         view->setResizeMode(QListView::ResizeMode::Fixed);
         view->setFlow(QListView::Flow::TopToBottom);
         view->setGridSize({ 0, 0 }); // this actually applies to List Mode, and so must be cleared when switching back from Icon Mode
         view->setSpacing(0);
         view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);
         view->setWordWrap(false);
         button->setIcon(style->standardIcon(QStyle::SP_FileDialogListView));
         break;
      case _::IconMode:
         view->setUniformItemSizes(false);
         view->setBatchSize(100);
         view->setResizeMode(QListView::ResizeMode::Adjust);
         view->setFlow(QListView::Flow::LeftToRight);
         view->setGridSize({ 64, 64 });
         view->setSpacing(2);
         view->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel); // necessary to fix Qt-side scroll speed issues in icon view
         view->setWordWrap(true); // TODO: not enough, on its own, to allow variable-height rows
         button->setIcon(style->standardIcon(QStyle::SP_FileDialogContentsView));
         break;
   }
   view->setUpdatesEnabled(true);
}
void DKBSABrowseDialog::upOneLevel() {
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return;
   auto index = view->rootIndex();
   if (!index.isValid())
      return;
   if (index == this->state.pathStemIndex) // don't allow navigating up above the stem
      return;
   auto parent = model->parent(index);
   view->setRootIndex(parent);
   emit this->directoryEntered(model->fullPathTo(parent));
}

void DKBSABrowseDialog::setBackend(DKBSACollectionModelBackend* backend) {
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (model) {
      QString prior = this->directory();
      //
      model->setBackend(backend);
      //
      // Setting the backend resets the model and root index, pulling us back to the root. 
      // We need to reacquire the path stem index and, if we were viewing a subfolder, 
      // navigate back into that subfolder.
      //
      this->state.pathStemIndex = model->indexOfFolder(this->state.pathStem);
      if (!prior.isEmpty()) {
         this->setDirectory(prior);
         return;
      }
      if (this->state.pathStemIndex.isValid()) {
         view->setRootIndex(this->state.pathStemIndex);
         emit this->directoryEntered(model->fullPathTo(this->state.pathStemIndex));
      }
   }
}
bool DKBSABrowseDialog::setDirectory(QString path) {
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return false;
   path = QDir::cleanPath(path);
   {
      const auto& stem = this->state.pathStem;
      if (!stem.isEmpty())
         if (!stem.startsWith(path))
            return false;
   }
   auto index = model->indexOfFolder(path);
   if (!index.isValid())
      return false;
   view->setRootIndex(index);
   emit this->directoryEntered(path);
   return true;
}
void DKBSABrowseDialog::setDirectoryAndFile(const QString& path) {
   auto ip = QDir::cleanPath(path);
   //
   if (!this->state.pathStem.isEmpty())
      if (!ip.startsWith(this->state.pathStem))
         return;
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (!model)
      return;
   //
   QString filename;
   auto i = ip.lastIndexOf('/');
   auto j = ip.lastIndexOf('.');
   if (j > i) {
      if (i > 0) {
         filename = ip.mid(i + 1);
         ip = ip.mid(0, i);
      } else {
         filename = ip;
         ip.clear();
      }
   }
   if (!this->setDirectory(ip))
      return;
   emit this->directoryEntered(ip);
   if (j > 0 && j > i) {
      auto* sm = view->selectionModel();
      if (!sm)
         return;
      auto index = model->indexOfFile(filename, view->rootIndex());
      if (!index.isValid())
         return;
      sm->setCurrentIndex(index, QItemSelectionModel::SelectionFlag::ClearAndSelect | QItemSelectionModel::SelectionFlag::Current);
   }
}
void DKBSABrowseDialog::setPathStem(const QString& stem) {
   if (stem == this->state.pathStem)
      return;
   auto& store = this->state.pathStem;
   store = QDir::cleanPath(stem);
   if (store.startsWith('/'))
      store.remove(0, 1);
   //
   auto* view  = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   if (model) {
      this->state.pathStemIndex = model->indexOfFolder(store);
      view->setRootIndex(this->state.pathStemIndex);
   } else {
      this->state.pathStemIndex = QModelIndex();
   }
}

void DKBSABrowseDialog::_updateFilenameTextFromSelection() {
   auto* view = this->subwidgets.view;
   auto* model = qobject_cast<DKBSACollectionModel*>(view->model());
   auto* sm = view->selectionModel();
   if (!model || !sm)
      return;
   QString names;
   auto    rows = sm->selectedRows(0);
   for (const auto& qmi : rows) {
      if (!model->isFile(qmi))
         continue;
      auto name = model->data(qmi, Qt::DisplayRole).toString();
      if (name.isEmpty())
         continue;
      if (rows.size() > 1) {
         names += QString("\"%1\"").arg(name);
      } else {
         names = name;
      }
   }
   this->subwidgets.filename->setText(names);
}