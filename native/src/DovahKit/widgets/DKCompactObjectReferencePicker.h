#pragma once
#if !defined(QT_PLUGIN)
   #include <functional>
#endif
#include <string>
#include <string_view>
#include <QPushButton>

#include "dovah/form_types.h"
#include "./DKFormPicker.h"

namespace dovah {
   class form_stub;
}

class DKCompactObjectReferencePicker : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(QString placeholder READ placeholder WRITE setPlaceholder DESIGNABLE true USER true);
   Q_PROPERTY(dovah::form_type requiredFormType READ requiredFormType WRITE setRequiredFormType DESIGNABLE true);
   public:
      DKCompactObjectReferencePicker(QWidget* parent = nullptr);

      constexpr bool allowNone() const noexcept { return true; }

      #if !defined(QT_PLUGIN)
         constexpr dovah::form_stub* ref() const noexcept {
            return this->state.value;
         }
      #endif

   public slots:
      inline QString placeholder() const { return this->state.placeholder; }
      void setPlaceholder(QString);

      #if !defined(QT_PLUGIN)
         void setRef(dovah::form_stub*);

         void setValidationFunction(std::function<bool(dovah::form_stub*)>&&);
         void setValidationFunction(const std::function<bool(dovah::form_stub*)>&);

         const std::string& requiredScriptname() const;
         void setRequiredScriptname(QString);
         void setRequiredScriptname(std::string_view);
      #endif

      constexpr dovah::form_type requiredFormType() const { return this->state.required_form_type; }
      void setRequiredFormType(dovah::form_type);

   signals:
      void refChanged(dovah::form_stub*);

   protected:
      QPushButton* _button = nullptr;
      struct {
         QString placeholder;

         dovah::form_type required_form_type = dovah::form_type::reference;
         std::string      required_scriptname;

         #if !defined(QT_PLUGIN)
            dovah::form_stub* value = nullptr;

            std::function<bool(dovah::form_stub*)> validation_function;
         #endif
      } state;

      void _update_text();
};