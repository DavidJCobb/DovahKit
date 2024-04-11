#pragma once
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/forms/structs/precached_nif_info.h"
   #include "ui/types/nif_for_form.h"
   #include "ui/types/nif_texture_swap.h"
#endif

#if !defined(QT_DESIGNER_LIB)
namespace dovah {
   namespace loaded_forms {
      namespace components {
         class model;
      }
      class Form;
   }
}
#endif

namespace dovah {
   class form_stub;
}

class DKFormNIFPicker : public QWidget {
   Q_OBJECT;
   public:
      DKFormNIFPicker(QWidget* parent = nullptr);

      #if !defined(QT_DESIGNER_LIB)
         void initializeFrom(const dovah::loaded_forms::components::model&);
         void commitTo(dovah::loaded_forms::components::model&, dovah::loaded_forms::Form& owner);

         constexpr const ui::types::nif_for_form& value() const { return this->_value; }
         void setValue(const ui::types::nif_for_form&);
      #endif

   signals:
      void dataChanged();

   protected:
      struct {
         QLineEdit*   path   = nullptr;
         QPushButton* button = nullptr;
      } _subwidgets;
      #if !defined(QT_DESIGNER_LIB)
         ui::types::nif_for_form _value;
      #endif
};