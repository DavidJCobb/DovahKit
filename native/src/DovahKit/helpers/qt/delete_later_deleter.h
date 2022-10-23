#pragma once
#include <type_traits>
#include <QObject>

namespace cobb::qt {
   template<typename Object> requires std::is_base_of_v<QObject, Object>
   struct delete_later_deleter {
      void operator()(Object* ptr) const {
         ptr->deleteLater();
      }
   };
}