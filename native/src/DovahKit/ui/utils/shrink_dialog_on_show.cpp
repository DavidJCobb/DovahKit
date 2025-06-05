#include "./shrink_dialog_on_show.h"
#include <cassert>
#include <QEvent>

class ShrinkDialogOnShowHelper : public QObject {
   public:
      ShrinkDialogOnShowHelper(QDialog& parent, bool width, bool height) : QObject(&parent), shrink_w(width), shrink_h(height) {
         parent.installEventFilter(this);
      }

      bool shrink_w = false;
      bool shrink_h = false;
      bool fired    = false;

      bool eventFilter(QObject* obj, QEvent* event) override {
         if (this->fired) {
            return QObject::eventFilter(obj, event);
         }
         if (event->type() == QEvent::Type::Show) {
            this->fired = true;
            obj->removeEventFilter(this);
            this->setParent(nullptr);
            this->deleteLater();

            QDialog* casted = qobject_cast<QDialog*>(obj);
            if (casted) {
               QSize size = casted->minimumSize();
               if (!this->shrink_w || !this->shrink_h) {
                  auto prior = casted->size();
                  if (!this->shrink_w) {
                     size.setWidth(prior.width());
                  }
                  if (!this->shrink_h) {
                     size.setHeight(prior.height());
                  }
               }
               casted->resize(size);
            }
         }
         return QObject::eventFilter(obj, event);
      }
};

namespace ui {
   extern void shrink_dialog_on_show(QDialog& d) {
      //
      // This looks like a leak, but isn't. The helper object is parented to `d` 
      // and will be destroyed when `d` is destroyed. If the helper object is 
      // activated before then, it'll orphan itself and destroy itself.
      //
      new ShrinkDialogOnShowHelper(d, true, true);
   }
   extern void shrink_dialog_height_on_show(QDialog& d) {
      new ShrinkDialogOnShowHelper(d, false, true);
   }
}