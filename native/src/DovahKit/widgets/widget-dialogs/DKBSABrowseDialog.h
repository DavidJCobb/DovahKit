#pragma once
#include <QDialog>
#include <QLineEdit>
#include <QListView>
#include <QToolButton>

class DKBSABrowseDialog : public QDialog {
   Q_OBJECT;
   public:
      DKBSABrowseDialog(QWidget* parent = nullptr);

      // The "minimum" file path; users can't select anything outside of this folder. Useful for 
      // engine-level limits, e.g. texture paths always being relative to "data/textures".
      inline QString pathStem() const noexcept { return this->state.pathStem; }

      static_assert(false, "add a static member function for selecting a file, compared to the static functions on QFileDialog");
         //  - need a stem, as indicated above
         //  - allow an already-selected file, for when the user clicks "browse" on an already-filled DKGameFilePicker
         //  - file extension(s) would be nice, but would require model-side changes

   signals:
      void fileSelected(const QString& file);

   public slots:
      void setPathStem(const QString&);

   protected:
      struct {
         QToolButton* upOneLevel = nullptr;
         QLineEdit*   path       = nullptr;
         QListView*   view       = nullptr;
         QLineEdit*   filename   = nullptr;
      } subwidgets;
      struct {
         QString     pathStem;
         QModelIndex pathStemIndex;
      } state;
};