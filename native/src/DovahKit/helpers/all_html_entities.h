#pragma once
#include <array>
#include <QString>

namespace cobb::html {
   struct entity_definition {
      QString name;
      QString replace;

      entity_definition() {}
      entity_definition(uint char_code, QString n) : name(n) {
         if (char_code <= 0xFFFF) {
            this->replace = QChar(char_code);
         } else {
            this->replace = QString::fromUcs4(&char_code, 1);
         }
      }
   };

   extern std::array<entity_definition, 2024> all_entities;

   extern QString look_up_entity(const QString& name);
}
