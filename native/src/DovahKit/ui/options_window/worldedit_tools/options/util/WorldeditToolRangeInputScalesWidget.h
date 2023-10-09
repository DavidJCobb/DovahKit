#pragma once
#include <optional>
#include "editor/subsystems/worldinput/enums/range_input_axes.h"
#include "editor/subsystems/worldinput/enums/range_input_control.h"
#include "editor/subsystems/worldinput/util/range_input_scales.h"
#include "ui_worldedit_tool_range_input_scales.h" // generated

class WorldeditToolRangeInputScalesWidget : public QWidget {
   Q_OBJECT;
   public:
      using data_type = dovahkit::subsystems::worldinput::util::range_input_scales;

   protected:
      using range_input_axes    = dovahkit::subsystems::worldinput::range_input_axes;
      using range_input_control = dovahkit::subsystems::worldinput::range_input_control;

   public:
      WorldeditToolRangeInputScalesWidget(QWidget* parent = nullptr);

      std::optional<data_type> data() const;
      void setData(const std::optional<data_type>&);
      void setData(const data_type&);

      bool isOptional() const;
      void setIsOptional(bool);

      QString label() const;
      void setLabel(const QString);

      std::optional<QString> axisNameOverride(data_type::axis3D) const;
      void clearAxisNameOverride(data_type::axis3D);
      void setAxisNameOverride(data_type::axis3D, QString);

      void reloadFromSyncTarget();
      void setSyncTarget(std::optional<data_type>*);

      void adjustForRangeInput(dovahkit::subsystems::worldinput::range_input_control, dovahkit::subsystems::worldinput::range_input_axes);

   protected:
      Ui::WorldeditToolRangeInputScalesWidget ui;
      struct {
         std::optional<QString> x;
         std::optional<QString> y;
         std::optional<QString> z;
      } overrides;
      std::optional<data_type>* sync_target = nullptr;

      const std::optional<QString>& _override_for_axis(data_type::axis3D) const;
      std::optional<QString>& _override_for_axis(data_type::axis3D);
};