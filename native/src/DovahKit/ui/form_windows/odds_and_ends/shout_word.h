#pragma once
#include <cstdint>
#include <QDialog>
#include "ui_shout_word.h"
#include "../../../dovah/form_stub.h"
#include "../../../dovah/forms/Shout.h"

class FormShoutWordEditor : public QWidget {
   Q_OBJECT;
   public:
      FormShoutWordEditor(QWidget* parent = Q_NULLPTR);
      //
      void initialize();
      void load();
      void save();
      inline DKFormPicker* spellCombobox() const noexcept { return this->ui.spell; }
      inline DKFormPicker* wordCombobox() const noexcept { return this->ui.word; }
      //
   private slots:
      //
   private:
      Ui::FormShoutWordEditor ui;
      int which_word = -1;
      dovah::form_stub* stub = nullptr;
      dovah::loaded_form_ptr<dovah::loaded_forms::Shout> form;
      //
      void save_form_id(dovah::form_reference_t& target, dovah::bare_form_id_t);
      void save_form_id(dovah::form_reference_t& target, dovah::form_stub*);
      //
   public:
      void linkToForm(int which_word, dovah::form_stub*);
};
