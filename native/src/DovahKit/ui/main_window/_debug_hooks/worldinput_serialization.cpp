#include "./worldinput_serialization.h"
#include <QDebug>

#include "editor/subsystems/worldinput2/core.h"
#include "editor/subsystems/worldinput2/control_schemes/ck_standard.h"
#include "editor/subsystems/worldinput2/control_schemes/debug_wasd.h"
#include "editor/subsystems/worldinput2/control_schemes/reach.h"

#include "editor/subsystems/worldinput2/bind_tree/tree.h"

#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

namespace {
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }
}
#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"

namespace DovahKitDebug::features {
   /*static*/ void worldinput_serialization::execute(QWidget* from) {
      auto& core = dovahkit::subsystems::worldinput2::core::get();

      qDebug("Running Worldinput control scheme serialization tests...");

      auto _compare = [](const char* name, const worldinput2::binds::tree& src) {
         cobb::bitstreams::writer writer;
         src.write(writer);

         cobb::bitstreams::reader reader;
         reader.set_buffer(writer.data(), writer.get_bytespan());
         const auto dst = worldinput2::binds::tree::read(reader);

         bool equal = (src == dst);

         qDebug("%s serialization worked? %s", name, equal ? "true" : "false");
         #if _DEBUG
            if (!equal) {
               __debugbreak();
            }
         #endif
      };
      _compare("CK Standard", worldinput2::default_control_schemes::ck_standard());
      _compare("Debug WASD",  worldinput2::default_control_schemes::debug_wasd());
      _compare("Reach",       worldinput2::default_control_schemes::reach());

      qDebug("All tests run.");
   }
}