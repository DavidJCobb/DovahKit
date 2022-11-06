#pragma once
#include <QObject>
#include "editor/subsystems/worldinput/inputs/bound_input.h"
#include "editor/subsystems/worldinput/tools/_base.h"

// QObject subclasses are the namespaces for Qt's compile-time localization system
// A QObject subclass doesn't have to actually be spawnable

class DKWorldinputLocalization : public QObject {
   Q_OBJECT;
   private:
      DKWorldinputLocalization() {}

   public:
      static QString stringify_input(const dovahkit::subsystems::worldinput::inputs::bound_input& bi);
      static QString tool_name(dovahkit::subsystems::worldinput::tool_id id);
      static QString tool_name(const dovahkit::subsystems::worldinput::tools::base*);
};