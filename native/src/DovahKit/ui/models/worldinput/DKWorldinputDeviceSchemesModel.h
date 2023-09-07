#pragma once
#include "ui/models/DKGenericListModel.h"
#include "editor/subsystems/worldinput2/enums/input_device_type.h"
#include "editor/subsystems/worldinput2/control_scheme.h"
#include "editor/subsystems/worldinput2/worldinput_control_scheme_manager.h"

class DKWorldinputDeviceSchemesModel;
struct DKWorldinputDeviceSchemesModelNode {
   using data_type       = dovahkit::subsystems::worldinput2::control_scheme;
   using saved_data_type = dovahkit::subsystems::worldinput2::control_scheme_manager::saved_control_scheme;

   const saved_data_type* src = nullptr;
};

class DKWorldinputDeviceSchemesModel : public DKGenericListModel<DKWorldinputDeviceSchemesModel, DKWorldinputDeviceSchemesModelNode> {
   Q_OBJECT;
   friend base_type;
   public:
      using input_device_type = dovahkit::subsystems::worldinput2::input_device_type;
      using saved_data_type   = node_type::saved_data_type;
      using data_type         = dovahkit::subsystems::worldinput2::control_scheme;

   public:
      static constexpr const auto IsHardcodedRole = (Qt::ItemDataRole)(Qt::UserRole + 1);

   #pragma region Stubs, to be overridden on the self type
   protected:
      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
   #pragma endregion

   protected:
      input_device_type _last_used_device_type = input_device_type::keyboard_mouse;

   public:
      DKWorldinputDeviceSchemesModel(QObject* parent = nullptr);

      void reload(input_device_type);

      int rowFor(const data_type&);

      std::optional<data_type> dataFor(const QModelIndex&) const;
      void replaceDataFor(const QModelIndex&, const data_type&);

      const saved_data_type* savedSchemeAtRow(int) const;

      QModelIndex insert(data_type);

      virtual bool removeRows(int row, int count, const QModelIndex& parent = {}) override;
};