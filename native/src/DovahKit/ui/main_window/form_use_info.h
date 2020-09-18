#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_form_use_info.h"

namespace dovah {
   class form_stub;
}

class FormUseInfoDialog : public QDialog {
   Q_OBJECT
   //
   public:
      FormUseInfoDialog(QWidget* parent) = delete;
      FormUseInfoDialog(const dovah::form_stub* stub, QWidget* parent = Q_NULLPTR); // needs to be public for Qt? but do not call; use the static getter
      //
   private slots:
      void rebuild();
      //
   private:
      Ui::FormUseInfoDialog ui;
      const dovah::form_stub* stub = nullptr;
};
