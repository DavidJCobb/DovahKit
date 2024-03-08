#pragma once
#include <string>
#include <string_view>
#include <QComboBox>
#include <QFlags>
#include <QWidget>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/Quest.h"
   #include "dovah/form_stub.h"
   #include "ui/types/quest_alias.h"
#else
   namespace dovah {
      namespace loaded_forms {
         class Quest;
      }
      class form_stub;
   }
   namespace ui::types {
      struct quest_alias;
   }
#endif
#include "./DKFormPicker.h"

class DKQuestAliasPicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(AliasTypes allowedTypes       READ allowedTypes       WRITE setAllowedTypes       DESIGNABLE true);
   Q_PROPERTY(QString    requiredScriptname READ requiredScriptname WRITE setRequiredScriptname DESIGNABLE true);
   public:
      enum class AliasType {
         // Values must be flags-masks, not bit indices/etc., for QFlags to work.
         Location  = 0x0001,
         Reference = 0x0002,

         Any = Location | Reference,
      };
      Q_DECLARE_FLAGS(AliasTypes, AliasType);

   public:
      DKQuestAliasPicker(QWidget* parent = nullptr);

      AliasTypes allowedTypes() const;
      void setAllowedTypes(AliasTypes);

      QString requiredScriptname() const;
      void setRequiredScriptname(QString);
      void setRequiredScriptname(std::string_view);

      #if !defined(QT_DESIGNER_LIB)
      ui::types::quest_alias questAlias() const;
      void setQuestAlias(const ui::types::quest_alias&);

      dovah::form_stub* quest() const;
      void setQuest(dovah::form_stub*);
      #endif

   signals:
      void aliasChanged(const ui::types::quest_alias&);
      void questChanged(dovah::form_stub* quest);

   protected:
      void _updateAliasList(bool allow_signals = true);

   protected:
      struct {
         AliasTypes  allowed_types = AliasType::Any;
         std::string required_scriptname;
         //
         #if !defined(QT_DESIGNER_LIB)
            dovah::form_stub* quest = nullptr;
            dovah::loaded_form_ptr<dovah::loaded_forms::Quest> quest_data;
         #endif
      } _state;
      struct {
         DKFormPicker* quest = nullptr;
         QComboBox*    alias = nullptr;
      } _subwidgets;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(DKQuestAliasPicker::AliasTypes);