#pragma once
#include <QPushButton>
#include "../core/subsystems/resources/DovahscriptResource.h"

class DovahscriptSaveButton : public QPushButton {
   Q_OBJECT;
   using DSRH = dovahscript::DovahscriptResourceUIHandle;
   public:
      DovahscriptSaveButton(QWidget* parent = nullptr);

      inline QString desiredFilename() const noexcept { return this->state.filename; }
      inline QString text() const noexcept { return this->state.text; }

      inline bool contentIsText() const noexcept { return !this->state.content.text.isEmpty(); }
      inline bool hasContent() const noexcept { return this->contentIsText() || (this->state.content.resource != nullptr); }

      inline DSRH resourceContent() const noexcept { return this->state.content.resource; }
      inline QString textContent() const noexcept { return this->state.content.text; }

      void setContent(DSRH);
      void setContent(const QString&);
      void setDesiredFilename(QString);
      void setText(const QString&);

   public slots:
      void requestSave();

   protected:
      struct {
         QString filename;
         QString text;
         struct {
            QString text;
            DSRH    resource;
         } content;
      } state;
};