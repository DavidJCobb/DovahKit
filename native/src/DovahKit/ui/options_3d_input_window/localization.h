#pragma once
#include <QObject>
#include "dk3d/inputs/bound_input.h"
#include "dk3d/tools/_base.h"

// QObject subclasses are the namespaces for Qt's compile-time localization system
// A QObject subclass doesn't have to actually be spawnable

class DK3DLocalization : public QObject {
   Q_OBJECT;
   private:
      DK3DLocalization() {}

   public:
      static QString stringify_input(const DK3D::inputs::bound_input& bi);
      static QString tool_name(DK3D::tool_id id);
      static QString tool_name(const DK3D::tools::base*);
};