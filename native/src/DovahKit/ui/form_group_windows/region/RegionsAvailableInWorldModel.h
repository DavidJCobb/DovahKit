#pragma once
#include <QAbstractItemModel>
namespace dovah {
   class form_stub;
}

class RegionsAvailableInWorldModel : public QAbstractItemModel {
   Q_OBJECT;
   public:
      RegionsAvailableInWorldModel(QObject* parent = nullptr);

   protected:
      struct Item {
         dovah::form_stub* stub = nullptr;
         QString editor_id;
      };
};