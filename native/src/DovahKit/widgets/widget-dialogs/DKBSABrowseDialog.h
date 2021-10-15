#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QListView>
#include <QStyledItemDelegate>
#include <QToolButton>

class DKBSACollectionModelBackend;

class DKBSABrowseDialogItemDelegate : public QStyledItemDelegate {
   public:
      using QStyledItemDelegate::QStyledItemDelegate;
   
      virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

class DKBSABrowseDialog : public QDialog {
   Q_OBJECT;
   public:
      DKBSABrowseDialog(QWidget* parent = nullptr);

      QString directory() const noexcept;

      // The "minimum" file path; users can't select anything outside of this folder. Useful for 
      // engine-level limits, e.g. texture paths always being relative to "data/textures".
      inline QString pathStem() const noexcept { return this->state.pathStem; }

      static QString getOpenFileName(
         QWidget* parent = nullptr,
         const QString& caption  = QString(),
         const QString& pathStem = QString(),
         const QString& initial  = QString(),
         const QString& filter   = QString(),
         QString* selectedFilter = nullptr,
         DKBSACollectionModelBackend* backend = nullptr
      );

   signals:
      void directoryEntered(const QString& path);
      void fileSelected(const QString& file);

   public slots:
      void setBackend(DKBSACollectionModelBackend*);
      bool setDirectory(QString);
      void setDirectoryAndFile(const QString& filePath);
      void setPathStem(const QString&);

   protected slots:
      void acceptWithFile(const QString&);
      void offerLooseFile(); // open a QFileDialog and limit it to the proper directory, if that directory exists
      void openNode(const QModelIndex&);
      void openSelectedNode(); // if the selected node is a folder, navigate into it; if it's a file, pick it and accept
      void selectFileByName(QString);
      void selectPath(const QString&);
      void setViewMode(QListView::ViewMode);
      void upOneLevel();

      void _updateFilenameTextFromSelection();
      void _updateIconColumnSpacing(QSize old, QSize now);

   protected:
      struct {
         QToolButton* upOneLevel = nullptr;
         QToolButton* viewMode   = nullptr;
         QLineEdit*   path       = nullptr;
         QListView*   view       = nullptr;
         QLineEdit*   filename   = nullptr;
      } subwidgets;
      struct {
         QString     pathStem;
         QModelIndex pathStemIndex;
         QString     _looseFilePath;
         QString     _finalResult;
         DKBSABrowseDialogItemDelegate* _delegate = nullptr;
      } state;

      virtual bool eventFilter(QObject* watched, QEvent* event);
};