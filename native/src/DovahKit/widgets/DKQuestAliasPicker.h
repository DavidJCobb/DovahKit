#pragma once
#include <string>
#include <string_view>
#include <QComboBox>
#include <QFlags>
#include <QWidget>
#if !defined(QT_PLUGIN)
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

      #if !defined(QT_PLUGIN)
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

      #if !defined(QT_PLUGIN)
      bool _wouldAllowQuest(const dovah::form_stub&) const;
      bool _wouldAllowQuestAlias(const ui::types::quest_alias&) const;
      
      struct _set_quest_options {
         bool emit_signals        = true;
         bool update_alias_list   = true;
         bool update_quest_picker = true;
         bool verify_quest_first  = false;
      };
      // returns true if the selected quest is actually changed:
      bool _setQuestImpl(dovah::form_stub* quest, _set_quest_options);
      #endif

   protected:
      struct {
         AliasTypes  allowed_types = AliasType::Any;
         std::string required_scriptname;
         //
         #if !defined(QT_PLUGIN)
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