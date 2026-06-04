#pragma once

namespace dovahkit::subsystems::game_localized_strings {
   enum class character_encoding {
      utf_8,
      windows_1250, // central/eastern european
      windows_1251, // cyrillic
      windows_1252, // western european
      windows_1253, // greek
      windows_1254, // turkish
      windows_1255,
      windows_1256, // arabic
      windows_1257,
      windows_1258,
   };
}