#pragma once
#include <vector>
#include <QObject>
#include <QPointer>

namespace dovah {
   class form_stub;
}
namespace ui::impl::DKFormPicker {
   class Model;
}
class DKFormPicker;

class DKFormPickerCustomFilter : public QObject {
   Q_OBJECT;
   friend class DKFormPicker;
   protected:
      // Functions provided for subclasses to call.
      void _refilter_form(const dovah::form_stub&);
      void _refilter_all_forms();

   // Fields to be managed by DKFormPicker widgets.
   private:
      std::vector<QPointer<ui::impl::DKFormPicker::Model>> _models;

      void _hookToModel(ui::impl::DKFormPicker::Model*);
      void _unhookFromModel(ui::impl::DKFormPicker::Model*);

   public:
      using QObject::QObject;

      virtual bool form_matches(const dovah::form_stub&) const noexcept = 0;
};