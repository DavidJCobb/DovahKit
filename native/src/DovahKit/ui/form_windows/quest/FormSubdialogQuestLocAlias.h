#pragma once
#include <QDialog>
#include "ui_FormSubdialogQuestLocAlias.h" // generated

namespace dovah::loaded_forms {
   class LocationAlias;
   class Quest;
}

class FormSubdialogQuestLocAlias : public QDialog {
   Q_OBJECT;
   public:
      using loaded_form_type  = dovah::loaded_forms::Quest;
      using loaded_alias_type = dovah::loaded_forms::LocationAlias;

   public:
      FormSubdialogQuestLocAlias(loaded_form_type&, loaded_alias_type&, QWidget* parent = nullptr);

      void load();
      void save();

   protected:
      Ui::FormSubdialogQuestLocAlias ui;
      struct {
         loaded_form_type&  quest;
         loaded_alias_type& alias;
      } _data;

      void _update_ext_alias_combobox();
};