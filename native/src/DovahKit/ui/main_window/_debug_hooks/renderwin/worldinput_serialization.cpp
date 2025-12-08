#include "./worldinput_serialization.h"
#include <QDebug>

#include "editor/subsystems/worldinput/core.h"
#include "editor/subsystems/worldinput/builtin_control_schemes/ck_standard.h"
#include "editor/subsystems/worldinput/builtin_control_schemes/debug_wasd.h"
#include "editor/subsystems/worldinput/builtin_control_schemes/reach.h"
#include "editor/subsystems/worldinput/control_scheme.h"

#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace {
   namespace worldinput {
      using namespace dovahkit::subsystems::worldinput;
   }
}
#include "editor/subsystems/worldinput/algorithms/input_sequence_stringification.h"

namespace DovahKitDebug::features::renderwin {
   /*static*/ void worldinput_serialization::execute(QWidget* from) {
      auto& core = worldinput::core::get();

      qDebug("Running Worldinput control scheme serialization tests...");

      auto _compare = [](const char* name, const worldinput::control_scheme& src) {
         cobb::bitstreams::writer writer;
         src.write(writer);

         cobb::bitstreams::reader reader;
         reader.set_buffer(writer.data(), writer.get_bytespan());
         const auto dst = worldinput::control_scheme::read(reader);

         bool equal = (src == dst);

         qDebug("%s serialization worked? %s", name, equal ? "true" : "false");
         #if _DEBUG
            if (!equal) {
               __debugbreak();
            }
         #endif
      };
      _compare("CK Standard", worldinput::builtin_control_schemes::ck_standard());
      _compare("Debug WASD",  worldinput::builtin_control_schemes::debug_wasd());
      _compare("Reach",       worldinput::builtin_control_schemes::reach());

      qDebug("All tests run.");
   }
}