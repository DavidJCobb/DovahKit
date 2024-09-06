#pragma once
#include <vector>
#include <QObject>
#include <QPointer>

namespace dovah {
   class form_stub;
}
class DKFormListPane;
class DKFormListPaneModel;

class DKFormListPaneCustomFilter : public QObject {
   Q_OBJECT;
   friend class DKFormListPane;
   protected:
      // Functions provided for subclasses to call.
      void _refilter_form(dovah::form_stub&);
      void _refilter_all_forms();

   // Fields to be managed by DKFormPicker widgets.
   private:
      std::vector<QPointer<DKFormListPaneModel>> _models;

      void _hookToModel(DKFormListPaneModel*);
      void _unhookFromModel(DKFormListPaneModel*);

   public:
      using QObject::QObject;

      virtual bool form_matches(dovah::form_stub&) const noexcept = 0;
};