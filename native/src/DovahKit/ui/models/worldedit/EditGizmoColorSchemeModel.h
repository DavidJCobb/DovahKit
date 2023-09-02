#pragma once
#include <QIcon>
#include <QString>
#include "ui/models/DKGenericListModel.h"
#include "editor/subsystems/worldedit/gizmo_colors/gizmo_color_scheme.h"

class EditGizmoColorSchemeModel;
struct EditGizmoColorSchemeModelNode {
   using data_type = dovahkit::subsystems::worldedit::gizmo_color_scheme;

   QString   name; // `data.name` is a UTF-8 std::string; better to do the conversion just once
   data_type data;
   bool is_hardcoded = false;

   QIcon icon;

   void update_icon();
};

class EditGizmoColorSchemeModel : public DKGenericListModel<EditGizmoColorSchemeModel, EditGizmoColorSchemeModelNode> {
   Q_OBJECT;
   friend base_type;
   public:
      using DKGenericListModel::DKGenericListModel; // constructor

      using data_type = node_type::data_type;

   public:
      static constexpr const auto IsHardcodedRole = (Qt::ItemDataRole)(Qt::UserRole + 1);

   #pragma region Stubs, to be overridden on the self type
   protected:
      QVariant      data_of(const node_type&, Qt::ItemDataRole, size_t column) const;
      Qt::ItemFlags flags_of(const node_type&, size_t column) const;
   #pragma endregion

   public:
      void reset_from_options();
      void force_replace_options(const QModelIndex& selected = {});

      std::optional<data_type> dataFor(const QModelIndex&) const;
      void replaceDataFor(const QModelIndex&, const data_type&);

      QModelIndex insert(const data_type&);
};