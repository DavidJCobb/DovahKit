#pragma once
#include <map>
#include <QList>
#include <QWidget>
#if !defined(QT_DESIGNER_LIB)
   #include <functional>
   #include "dovah/core.h"
#endif
#include "dovah/form_types.h"
#if !defined(QT_DESIGNER_LIB)
   #include "./widget-data/DKFormPickerCustomFilter.h"
#endif

namespace dovah {
   class form_stub;
}
namespace ui::impl::DKFormPicker {
   class Model;
}
class DKComboBox;
class QComboBox;

class DKFormPicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(QList<dovah::form_type> allowedFormTypes        READ allowedFormTypes        WRITE setAllowedFormTypes        DESIGNABLE true USER true);
   Q_PROPERTY(bool    allowNone               READ allowNone               WRITE setAllowNone               DESIGNABLE true);
   Q_PROPERTY(QString overrideTextForNone     READ overrideTextForNone     WRITE setOverrideTextForNone     DESIGNABLE true);
   Q_PROPERTY(QString requiredScriptname      READ requiredScriptname      WRITE setRequiredScriptname      DESIGNABLE true);
   Q_PROPERTY(QString requiredAliasScriptname READ requiredAliasScriptname WRITE setRequiredAliasScriptname DESIGNABLE true);
   Q_PROPERTY(bool    splitTypesWhenMany      READ splitTypesWhenMany      WRITE setSplitTypesWhenMany      DESIGNABLE true);
   public:
      #if !defined(QT_DESIGNER_LIB)
         using ExtraFilterFunction = std::function<bool(const dovah::form_stub*)>;
      #endif
   public:
      DKFormPicker(QWidget* parent = nullptr);
      
      constexpr const QList<dovah::form_type>& allowedFormTypes() const noexcept { return this->_properties.allowed_form_types; }
      void setAllowedFormTypes(QList<dovah::form_type>) noexcept;
      //
      void addAllowedFormType(dovah::form_type);
      inline void allowAllFormTypes() noexcept { this->setAllowedFormTypes({}); }
      inline void setAllowedFormType(dovah::form_type ft) noexcept { this->setAllowedFormTypes({ ft }); }
      //
      inline bool allowsFormType(dovah::form_type ft) const noexcept {
         return this->_properties.allowed_form_types.contains(ft);
      }

      constexpr bool allowNone() const noexcept { return this->_properties.allow_none; }
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears

      QString overrideTextForNone() const;
      void setOverrideTextForNone(QString);

      QString requiredScriptname() const;
      void setRequiredScriptname(QString);
      void setRequiredScriptname(std::string_view);

      // For when the form-picker is used to select a quest. No effect when the form type 
      // being filtered for is not a quest.
      QString requiredAliasScriptname() const;
      void setRequiredAliasScriptname(QString);
      void setRequiredAliasScriptname(std::string_view);

      constexpr bool splitTypesWhenMany() const noexcept { return this->_properties.split_types_when_many; }
      void setSplitTypesWhenMany(bool) noexcept;

      #if !defined(QT_DESIGNER_LIB)
         constexpr dovah::form_stub* formStub() const noexcept { return this->_value; }
         void setFormStub(dovah::form_stub*) noexcept;

         constexpr dovah::form_stub* defaultForm() const noexcept { return this->_default; }
         void setDefaultForm(dovah::form_stub*) noexcept;
      #endif

      constexpr bool isSplittingTypes() const noexcept { return this->_state.is_splitting_types; }

      #if !defined(QT_DESIGNER_LIB)
         DKFormPickerCustomFilter* customFilter() const;
         void setCustomFilter(DKFormPickerCustomFilter* v);
      #endif

   protected:
      #if !defined(QT_DESIGNER_LIB)
         dovah::form_stub* _value   = nullptr;
         dovah::form_stub* _default = nullptr;
      #endif
      struct {
         bool allow_none = true;
         QList<dovah::form_type> allowed_form_types;
         bool split_types_when_many = true;

         QString scriptname_on_alias;
         QString scriptname_on_form;

         #if defined(QT_DESIGNER_LIB)
            QString override_text_for_none;
         #endif
      } _properties;
      struct {
         bool is_splitting_types = true;
         bool needs_initial_fill = false;

         // DKFormPickers choose not to list certain forms, e.g. unnamed exterior cells. 
         // However, there are some cases where we need to be able to programmatically 
         // set a DKFormPicker's current value to such a form and have that respected, 
         // so our workaround is to force-prepend such forms when set.
         dovah::form_stub* last_force_included_form = nullptr;
      } _state;
      struct {
         DKComboBox* form = nullptr;
         QComboBox*  type = nullptr;
      } _subwidgets;
      #if !defined(QT_DESIGNER_LIB)
         //
         // When the widget is set to split by form type (i.e. a combobox for form type and 
         // a combobox for form), we want to remember the user's last selection for each 
         // form type, so that if they change the form type combobox back and forth, they 
         // don't lose their selected form.
         //
         std::map<dovah::form_type, dovah::form_stub*> _prior_selections;
      #endif

      #if !defined(QT_DESIGNER_LIB)
         ui::impl::DKFormPicker::Model* _rawModel() const noexcept;

         bool _wouldAllowFormStub(const dovah::form_stub&) const;

         virtual void changeEvent(QEvent* event) override;
      #endif

      void _setIsSplittingTypes(bool) noexcept;
      void _setSubwidgetEnableState(bool);
      bool _shouldSplitTypes() const noexcept;
      void _updateForceIncludedForm(dovah::form_stub*);
      void _updateForms();
      void _updateTypePicker();
      
   signals:
      void formChanged(dovah::form_stub* selected);
      void populated();

   public slots:
      void clear();
};