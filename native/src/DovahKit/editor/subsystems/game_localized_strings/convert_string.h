#pragma once
#include <string>
#include <QString>
#include "./character_encoding.h"

namespace dovahkit::subsystems::game_localized_strings {
   extern QString convert_narrow_to_qt(std::string_view, character_encoding);
   extern std::string convert_qt_to_narrow(QString, character_encoding);
}