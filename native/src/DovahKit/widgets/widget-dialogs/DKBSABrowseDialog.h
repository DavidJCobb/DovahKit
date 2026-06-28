#pragma once
#if defined(QT_PLUGIN)
   #error This dialog relies on DovahKit to run (dependency in DKBSACollectionModelBackend). Do not include it when compiling the Qt Designer plug-in.
#endif
#include <expected>
#include <QDialog>
#include <QLineEdit>
#include <QListView>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QToolButton>
#include "../DKGameFilePicker.h"

class DKBreadcrumbBar;
class DKBSACollectionModel;
class DKBSACollectionModelBackend;

class DKBSABrowseDialogItemDelegate : public QStyledItemDelegate {
   public:
      using QStyledItemDelegate::QStyledItemDelegate;
   
      virtual void initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const override;
};

class DKBSABrowseDialog : public QDialog {
   Q_OBJECT;
   public:
      enum PathWarning {
           ArmorAddonSuffixMissing      = 0x01, // This path is for an ArmorAddon mesh, but the filename doesn't end in _0 or _1.
           ArmorAddonSuffixConfusable   = 0x02, // This path is for an ArmorAddon mesh, but the filename doesn't end in _0 or _1, and contains other underscores.
           DoubleDataDirectory          = 0x04, // This path begins with "data/data" when including the base Data folder.
           FileExtensionTooLong         = 0x08, // This file's extension is longer than 9 bytes.
           FirstFolderIsDataSuperstring = 0x10, // This path begins with a folder whose name starts with "data", e.g. "data/dataaaa" including the base Data folder.
           NoFileExtension              = 0x20, // This file has no extension, and may not reliably be recognized by the game as a file.
           NonASCIIPathComponent        = 0x40, // This path contains non-ASCII characters. Lookups from inside of a BSA will not work reliably.
           PeriodInFolderName           = 0x80, // Folders in this path contain periods in their names.
       };
      Q_DECLARE_FLAGS(PathWarnings, PathWarning);
      Q_FLAG(PathWarnings);

      using SpecialValidation = DKGameFilePicker::SpecialValidation;

      // Options struct for passing BSA-specific dialog options to the static member functions.
      struct DialogOptions {
         DKBSACollectionModelBackend* backend = nullptr;
         SpecialValidation specialValidation = SpecialValidation::NoSpecialValidation;
      };

   public:
      DKBSABrowseDialog(QWidget* parent = nullptr);

      // Path SHOULD NOT include the Data directory.
      static PathWarnings checkPath(const QString&, SpecialValidation);

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
         DialogOptions extra = DialogOptions()
      );

   signals:
      void directoryEntered(const QString& path);
      void fileSelected(const QString& file);

   public slots:
      void setBackend(DKBSACollectionModelBackend*);
      bool setDirectory(QString);
      void setDirectoryAndFile(const QString& filePath);
      void setPathStem(const QString&);
      void setSpecialValidation(SpecialValidation);

   protected slots:
      void acceptWithFile(const QString&);
      void offerLooseFile(); // open a QFileDialog and limit it to the proper directory, if that directory exists
      void openNode(const QModelIndex&);
      void openSelectedNode(); // if the selected node is a folder, navigate into it; if it's a file, pick it and accept
      void cycleViewMode();
      void setViewMode(QListView::ViewMode);
      void upOneLevel();

   protected:

      enum class path_resolution_failure {
         folder_does_not_exist,
         file_does_not_exist,
      };
      std::expected<QModelIndex, path_resolution_failure> _try_resolve_path(const QModelIndex& relative_to, const QList<QStringView>& path_segments);

      void _breadcrumb_qmi_changed(const QModelIndex&);
      QModelIndex _current_directory_qmi() const noexcept;
      QModelIndex _selected_node() const;
      void _set_current_directory_qmi(const QModelIndex&);
      void _set_current_directory_qmi(const QModelIndex&, QString path);
      void _try_navigate(QString);

      void _updateFilenameTextFromSelection();
      void _updateIconColumnSpacing(QSize old, QSize now);

   protected:
      DKBSACollectionModel* model = nullptr;
      struct {
         QToolButton*     upOneLevel = nullptr;
         QToolButton*     viewMode   = nullptr;
         DKBreadcrumbBar* path       = nullptr;
         QListView*       view       = nullptr;
         QLineEdit*       filename   = nullptr;
         QPushButton*     pickLoose  = nullptr;
         QPushButton*     buttonOK   = nullptr;
         QPushButton*     buttonQuit = nullptr;
      } subwidgets;
      struct {
         QString     pathStem;
         QModelIndex pathStemIndex;
         SpecialValidation specialValidation = SpecialValidation::NoSpecialValidation;
         QString     _looseFilePath;
         QString     _finalResult;
      } state;
      struct {
         QAbstractItemDelegate* list = nullptr;
         DKBSABrowseDialogItemDelegate* icon = nullptr;
      } item_delegates;

      virtual bool eventFilter(QObject* watched, QEvent* event) override;
      virtual void keyPressEvent(QKeyEvent*) override; // disable QDialog "auto-default" functionality
};