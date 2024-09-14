#pragma once
#include "./_base.h"
#include "dovah/forms/Topic.h"
#include "ui_topic.h" // generated

#include <optional>
#include "dovah/data/dialogue/category.h"

class FormDialogTopic :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Topic, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogTopic(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogTopic ui;
      std::optional<dovah::dialogue::category> _initial_category;
      struct {
         std::vector<dovah::form_stub*> sibling_topics;
      } _subtypes;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      static QString _subtype_name(size_t subtype_index);

      void _update_available_subtypes();
};
