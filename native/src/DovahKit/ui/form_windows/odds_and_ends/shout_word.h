#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_shout_word.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Shout.h"

class FormShoutWordEditor : public QWidget {
   Q_OBJECT
   //
   public:
      FormShoutWordEditor(QWidget* parent = Q_NULLPTR);
      //
      void initialize();
      void load();
      void save();
      inline FormsOfTypeCombobox* spellCombobox() const noexcept { return this->ui.spell; }
      inline FormsOfTypeCombobox* wordCombobox() const noexcept { return this->ui.word; }
      //
   private slots:
      //
   private:
      Ui::FormShoutWordEditor ui;
      int which_word = -1;
      dovah::loaded_form_ptr<dovah::loaded_forms::Shout> form;
      //
   public:
      void linkToForm(int which_word, dovah::form_stub*);
};
