#pragma once
#include <vector>
#include <QObject>
#include <QPointer>

namespace dovah {
   class form_stub;
}
class DKCustomFormFilterableModelMixin;

class DKCustomFormFilter : public QObject {
   Q_OBJECT;
   friend DKCustomFormFilterableModelMixin;
   public:
      using QObject::QObject;

      virtual bool form_matches(dovah::form_stub&) const noexcept = 0;

   protected:
      // Functions provided for subclasses to call.
      void _refilter_form(dovah::form_stub&);
      void _refilter_all_forms();

   private:
      std::vector<QPointer<QObject>> _models;

      void _hook_to_model(DKCustomFormFilterableModelMixin*);
      void _unhook_from_model(DKCustomFormFilterableModelMixin*);

};