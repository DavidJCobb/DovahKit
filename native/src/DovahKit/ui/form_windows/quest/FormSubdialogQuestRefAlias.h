#pragma once
#include <QButtonGroup>
#include <QDialog>
#include "ui_FormSubdialogQuestRefAlias.h" // generated

namespace dovah::loaded_forms {
   class ReferenceAlias;
   class Quest;
}

class FormSubdialogQuestRefAlias : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type  = dovah::loaded_forms::Quest;
      using loaded_alias_type = dovah::loaded_forms::ReferenceAlias;

   public:
      FormSubdialogQuestRefAlias(loaded_form_type&, loaded_alias_type&, QWidget* parent = nullptr);

      void load();
      void save();

   protected:
      Ui::FormSubdialogQuestRefAlias ui;
      QButtonGroup _fill_types;
      QButtonGroup _match_types;
      struct {
         loaded_form_type&  quest;
         loaded_alias_type& alias;
      } _data;

      void _update_enable_states();

      void _update_ext_alias_combobox();
};