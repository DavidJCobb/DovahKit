#pragma once
#include <cstdint>
#include <QList>
#include <QWidget>
#if !defined(QT_PLUGIN)
   #include <functional>
#endif

namespace dovah {
   class form_stub;
}
namespace ui::impl::DKTopicOrSubtypePicker {
   class Model;
}
class DKComboBox;
#if !defined(QT_PLUGIN)
   class DKCustomFormFilter;
#endif

class DKTopicOrSubtypePicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(bool    allowNone           READ allowNone           WRITE setAllowNone           DESIGNABLE true);
   Q_PROPERTY(QString overrideTextForNone READ overrideTextForNone WRITE setOverrideTextForNone DESIGNABLE true);
   public:
      #if !defined(QT_PLUGIN)
         using ExtraFilterFunction = std::function<bool(const dovah::form_stub*)>;
      #endif
   public:
      DKTopicOrSubtypePicker(QWidget* parent = nullptr);
      
      constexpr bool allowNone() const noexcept { return this->_properties.allow_none; }
      void setAllowNone(bool) noexcept; // set whether a "NONE" option appears

      QString overrideTextForNone() const;
      void setOverrideTextForNone(QString);

      #if !defined(QT_PLUGIN)
         constexpr dovah::form_stub* topic() const noexcept { return this->_topic; }
         void setTopic(dovah::form_stub*) noexcept;

         constexpr uint32_t subtype() const noexcept {
            if (!this->_topic)
               return this->_subtype;
            return 0;
         }
         void setSubtype(uint32_t);
      #endif

      #if !defined(QT_PLUGIN)
         DKCustomFormFilter* customFilter() const;
         void setCustomFilter(DKCustomFormFilter* v);
      #endif

   protected:
      #if !defined(QT_PLUGIN)
         dovah::form_stub* _topic   = nullptr;
         uint32_t          _subtype = 0;
      #endif
      struct {
         bool allow_none = true;
         #if defined(QT_PLUGIN)
            QString override_text_for_none;
         #endif
      } _properties;
      struct {
         bool needs_initial_fill = true;

         // DKTopicOrSubtypePickers choose not to list certain forms, e.g. unnamed exterior cells. 
         // However, there are some cases where we need to be able to programmatically 
         // set a DKTopicOrSubtypePickers's current value to such a form and have that respected, 
         // so our workaround is to force-prepend such forms when set.
         dovah::form_stub* last_force_included_form = nullptr;
      } _state;
      struct {
         DKComboBox* form = nullptr;
      } _subwidgets;

      #if !defined(QT_PLUGIN)
         ui::impl::DKTopicOrSubtypePicker::Model* _rawModel() const noexcept;

         bool _wouldAllowFormStub(const dovah::form_stub&) const;

         virtual void showEvent(QShowEvent* event) override;
      #endif

      void _setSubwidgetEnableState(bool);
      void _updateForceIncludedForm(dovah::form_stub*);
      void _updateForms();
      
   signals:
      void valueChanged(dovah::form_stub*, uint32_t);
      void populated();

   public slots:
      void clear();
};