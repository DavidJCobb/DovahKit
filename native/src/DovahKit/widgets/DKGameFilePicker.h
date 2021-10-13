#pragma once
#include <QLineEdit>
#include <QPushButton>

class DKGameFilePicker : public QWidget {
   Q_OBJECT;
   public:
      DKGameFilePicker(QWidget* parent);

      inline bool isEmpty() const noexcept { return this->state.value.isEmpty(); }
      inline QString path() const noexcept { return this->state.value; }
      inline QString stem() const noexcept { return this->state.stem; }

   public slots:
      void setPath(const QString&);
      void setStem(const QString&); // does not retroactively update the current path

   signals:
      void pathChanged(const QString&);

   protected:
      struct {
         QPushButton* browse = nullptr;
         QLineEdit*   path   = nullptr;
      } subwidgets;
      struct {
         QString stem; // e.g. to require a texture, you'd set this to "textures/"
         QString value;
      } state;
};