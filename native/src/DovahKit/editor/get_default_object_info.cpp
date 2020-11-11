#include "get_default_object_info.h"
#include <array>
#include <QObject>
#include <QString>

namespace {
   constexpr char* dis_n = "DOBJ name";
   constexpr char* dis_d = "DOBJ description";

   struct _info {
      uint32_t signature = 0;
      QString  name;
      QString  description;

      _info(uint32_t s, const QString& n, const QString& d) : signature(s), name(n), description(d) {}
      _info(uint32_t s, const QString& n) : signature(s), name(n) {}
   };
   std::array _default_objects = {
      _info('AWWW', QObject::tr("Hare Faction", dis_n), QObject::tr("A Faction to which all hares should belong. Used to help manage the \"Bunnies Slaughtered\" misc stat.", dis_d)),
      _info('DFTS', QObject::tr("Default Footstep Set", dis_n)),
      _info('SALT', QObject::tr("Sitting Angle Limit", dis_n)),
   };
}
extern QString get_default_object_name(uint32_t signature) {
   if (!signature)
      return QString();
   for (auto& entry : _default_objects) {
      if (entry.signature == signature)
         return entry.name;
   }
   return QString();
}
extern QString get_default_object_description(uint32_t signature) {
   if (!signature)
      return QString();
   for (auto& entry : _default_objects) {
      if (entry.signature == signature)
         return entry.description;
   }
   return QString();
}