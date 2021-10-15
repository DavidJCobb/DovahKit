#include "DKBSABrowseDialog.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDir>
#include <QFileDialog>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QResizeEvent>
#include <QToolBar>
#include "../widget-models/DKBSACollectionModel.h"
#include "../../editor/core.h"

//
// NOTE: Paths in BSAs are retained in-memory as lowercase, and they're represented 
// in the model backend the same way. However, the model uses case-sensitive searches 
// because those are faster. Accordingly, if we're taking a path or filename from the 
// "outside world," we have to force it to lowercase (and use QDir::cleanPath when 
// applicable).
//

namespace {
   //
   // In the Windows 10 shell, if there are enough icons in an icon-view listing to wrap 
   // onto multiple rows, then Windows inserts padding between columns in order to ensure 
   // that there isn't empty space on the righthand side of the window. This constexpr 
   // bool controls whether we mimic that behavior.
   // 
   // We do this by expanding our QListView's gridSize in order to fill the empty space, 
   // counting on the fact that items will be centered horizontally within their grid 
   // spaces. This does introduce an issue: if we're making items wider, then text will 
   // wrap and elide differently. We mitigate that with a QStyledItemDelegate subclass, 
   // which forcibly constrains items' rects, counting on the fact that the gridSize 
   // height is always the desired square dimensions.
   // 
   // We of course change the gridSize when the list view is resized; we listen for that 
   // using an eventFilter.
   // 
   // The effect is slightly jerky due to the fact that we're stuck using integer sizes; 
   // there may be small bits of extra space on the righthand side for the same reason. 
   // If we were using a custom QAbstractItemView subclass, we could use floating-point 
   // positions and simply "carry" subpixels over from item to item until we come up 
   // with a whole pixel, thereby ensuring that integer division doesn't cause us to 
   // "lose" pixels. Alas, that's not possible here.
   //
   constexpr bool padding_between_icon_columns = true;
}

void DKBSABrowseDialogItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
   QStyledItemDelegate::initStyleOption(option, index);
   //
   if constexpr (padding_between_icon_columns) {
      auto& rect = option->rect;
      auto  h    = rect.height();
      auto  w    = rect.width();
      if (h && h < w) {
         auto d = (w - h) / 2;
         rect.setWidth(h);
         rect.moveLeft(rect.x() + d);
      }
   }
}

DKBSABrowseDialog::DKBSABrowseDialog(QWidget* parent) : QDialog(parent) {
   auto* style = QApplication::style();
   {
      std::filesystem::path path;
      //
      auto& editor = DovahKitCore::get();
      editor.get_game_path(path, editor.get_current_game());
      //
      this->state._looseFilePath = QDir::cleanPath(QString::fromUtf8((const char*)path.u8string().c_str()) + "/Data");
      if (QDir(this->state._looseFilePath).exists() == false) {
         this->state._looseFilePath.clear();
      }
   }
   //
   this->setSizeGripEnabled(true);
   this->resize(600, 250);
   this->setMinimumSize(400, 150);
   //
   auto* path = this->subwidgets.path     = new QLineEdit(this);
   auto* view = this->subwidgets.view     = new QListView(this);
   auto* name = this->subwidgets.filename = new QLineEdit(this);
   {
      auto* layout = new QVBoxLayout(this);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      layout->setSpacing(0);
      {
         auto* toolbar = new QToolBar(this);
         layout->setMenuBar(toolbar);
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
      layout->addWidget(view, 1);
      {
         auto* bottom = new QVBoxLayout(this);
         bottom->setContentsMargins({
            style->pixelMetric(QStyle::PM_LayoutLeftMargin),
            style->pixelMetric(QStyle::PM_LayoutTopMargin),
            std::max(style->pixelMetric(QStyle::PM_LayoutRightMargin), style->pixelMetric(QStyle::PM_SizeGripSize)),
            style->pixelMetric(QStyle::PM_LayoutBottomMargin),
         });
         bottom->setSpacing(style->pixelMetric(QStyle::PM_LayoutVerticalSpacing));
         layout->addLayout(bottom, 0);
         //
         {  // Filename row
            auto* label  = new QLabel(tr("File &name:"), this);
            label->setBuddy(name);
            //
            auto* nested = new QHBoxLayout(this);
            nested->addSpacing(100);
            nested->addWidget(label);
            nested->addWidget(name);
            bottom->addLayout(nested, 0);
         }
         {  // Buttons row
            auto* nested = new QHBoxLayout(this);
            auto* loose  = this->subwidgets.pickLoose  = new QPushButton(tr("Loose file..."), this);
            auto* ok     = this->subwidgets.buttonOK   = new QPushButton(tr("&Open"), this);
            auto* cancel = this->subwidgets.buttonQuit = new QPushButton(tr("Cancel"), this);
            nested->addStretch(0);
            nested->addWidget(loose);
            nested->addWidget(ok);
            nested->addWidget(cancel);
            bottom->addLayout(nested, 0);
            //
            if (this->state._looseFilePath.isEmpty()) {
               loose->setEnabled(false);
            } else {
               QObject::connect(loose, &QPushButton::clicked, this, &DKBSABrowseDialog::offerLooseFile);
            }
            QObject::connect(ok,     &QPushButton::clicked, this, &DKBSABrowseDialog::openSelectedNode);
            QObject::connect(cancel, &QPushButton::clicked, this, &QDialog::reject);
         }
      }
      QWidget::setTabOrder(this->subwidgets.upOneLevel, this->subwidgets.path);
      QWidget::setTabOrder(this->subwidgets.path,       this->subwidgets.viewMode);
      QWidget::setTabOrder(this->subwidgets.viewMode,   this->subwidgets.view);
      QWidget::setTabOrder(this->subwidgets.view,       this->subwidgets.filename);
      QWidget::setTabOrder(this->subwidgets.filename,   this->subwidgets.pickLoose);
      QWidget::setTabOrder(this->subwidgets.pickLoose,  this->subwidgets.buttonOK);
      QWidget::setTabOrder(this->subwidgets.buttonOK,   this->subwidgets.buttonQuit);
   }
   //
   auto* model = new DKBSACollectionModel(this);
   view->setSpacing(0);
   view->setUniformItemSizes(true);
   view->setBatchSize(200);
   view->setLayoutMode(QListView::LayoutMode::Batched);
   view->setModel(model);
   view->installEventFilter(this);
   this->state._delegate = new DKBSABrowseDialogItemDelegate(view);
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
   // Navigation/selection events:
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
   QString loose_base = this->state._looseFilePath;
   QString loose_stem = QDir::cleanPath(loose_base + '/' + this->state.pathStem);
   if (!QDir(loose_base).exists()) {
      //
      // Handle the case of the Data directory being deleted at some point after this dialog 
      // was opened (why would the user do that??).
      //
      QMessageBox::critical(this, tr("Error: no loose file directory"), tr("Loose files are supposed to be stored in the following folder, but it doesn't seem to exist on your system:\n\n%1").arg(this->state._looseFilePath));
      return;
   }
   //
   auto dialog = new QFileDialog(this);
   dialog->setOptions(QFileDialog::Option::DontResolveSymlinks | QFileDialog::Option::ReadOnly);
   if (QDir(loose_stem).exists())
      dialog->setDirectory(loose_stem);
   else
      dialog->setDirectory(loose_base);
   dialog->setWindowModality(Qt::WindowModality::WindowModal);
   dialog->setWindowTitle(tr("Select loose file..."));
   QObject::connect(dialog, &QFileDialog::directoryEntered, this, [dialog, loose_base, loose_stem](const QString& entered) {
      //
      // Unfortunately, QFileDialog relies on WinRT to show the native Windows file dialog. 
      // From what I've been able to find in a brief search, the relevant WinRT APIs are 
      // far more limited than the classic Win32 APIs.
      // 
      // If we wanted to have an Open File dialog that is constrained to the Data directory 
      // (or better yet: the path stem within the Data directory), then we'd likely want to 
      // use the IFileOpenDialog interface in the Win32 APIs. It's possible to hook event 
      // listeners to the dialog; we'd want IFileDialogEvents::OnFolderChanging, which can 
      // be used to intercept and potentially prevent navigation to a given folder. However, 
      // we'd then have to implement basically all communication with the file dialog -- and
      // implement it in a manner that would allow a smooth fallback to QFileDialog on other 
      // platforms.
      // 
      // Not worth it at this time.
      //
      /*// Unfortunately, this code is not sufficient for what we wish to do.
      auto dir = QDir(loose_stem);
      if (dir.exists()) {
         if (dir.relativeFilePath(entered).startsWith("../")) {
            dialog->setDirectory(dir); // doesn't work on Windows
            QApplication::beep();
            return;
         }
      }
      dir = QDir(loose_base);
      if (dir.exists()) {
         if (dir.relativeFilePath(entered).startsWith("../")) {
            dialog->setDirectory(dir); // doesn't work on Windows
            QApplication::beep();
            return;
         }
      }
      //*/
   });
   QObject::connect(dialog, &QFileDialog::fileSelected, this, [this, loose_stem](const QString& path) {
      if (path.isEmpty())
         return;
      auto rel = QDir(loose_stem).relativeFilePath(path);
      if (rel.startsWith("../")) {
         QMessageBox::critical(this, tr("Error"), tr("You cannot select a file outside of Data/%1.").arg(this->state.pathStem));
         return;
      }
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
   QModelIndex file  = model->indexOfFile(name.toLower(), index);
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
   auto cleaned = QDir::cleanPath(path).toLower();
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
         view->setItemDelegate(nullptr);
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
         view->setItemDelegate(this->state._delegate);
         this->_updateIconColumnSpacing(QSize(), QSize());
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
   path = QDir::cleanPath(path).toLower();
   {
      const auto& stem = this->state.pathStem;
      if (!stem.isEmpty())
         if (!path.startsWith(stem))
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
   auto ip = QDir::cleanPath(path).toLower();
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
   auto& store = this->state.pathStem;
   {
      auto s = stem.toLower();
      if (s == this->state.pathStem)
         return;
      store = QDir::cleanPath(s);
      if (store.startsWith('/'))
         store.remove(0, 1);
   }
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
void DKBSABrowseDialog::_updateIconColumnSpacing(QSize old, QSize now) {
   auto* view = this->subwidgets.view;
   if (view->viewMode() != QListView::ViewMode::IconMode)
      return;
   auto* model = view->model();
   if (!model)
      return;
   auto  grid     = view->gridSize();
   auto  per_item = grid.height();
   auto  index    = view->rootIndex();
   auto  count    = model->rowCount(index);
   if (count <= 1) {
      view->setGridSize({ per_item, per_item });
      return;
   }
   //
   QSize size = now - old;
   if (auto* port = view->viewport()) {
      size += port->size();
   } else {
      size += view->contentsRect().size();
   }
   auto space = view->spacing();
   if (count * per_item + (count + 1 * space) <= size.width()) {
      view->setGridSize({ per_item, per_item });
      return;
   }
   //
   auto per_row = size.width() / per_item;
   if (per_row <= 1) {
      view->setGridSize({ per_item, per_item });
      return;
   }
   auto extra = size.width() - (per_row * per_item);
   extra -= (per_row + 1) * space;
   extra /= (per_row);
   view->setGridSize({ per_item + extra, per_item });
}

bool DKBSABrowseDialog::eventFilter(QObject* watched, QEvent* event) {
   if constexpr (padding_between_icon_columns) {
      if (event->type() == QEvent::Type::Resize && watched == this->subwidgets.view) {
         auto* casted = (QResizeEvent*)event;
         //
         this->_updateIconColumnSpacing(casted->oldSize(), casted->size());
         return false;
      }
   }
   return false;
}