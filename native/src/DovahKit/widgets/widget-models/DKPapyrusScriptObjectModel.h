#pragma once
#if defined(QT_DESIGNER_LIB)
   #error This model relies on DovahKit to run. Do not include it when compiling the Qt Designer plug-in.
#endif
#include <QAbstractItemModel>
#include <QMap>
#include <QString>
#include "dovah/forms/components/papyrus.h"

namespace dovah {
   class form_stub;
}

class DKPapyrusScriptObjectModel : public QAbstractTableModel {
   Q_OBJECT;
   public:
      using papyrus_attachment_data = dovah::loaded_forms::components::papyrus_attachment_data;
      using vmad_script   = dovah::loaded_forms::components::papyrus::attached_script;
      using vmad_property = dovah::loaded_forms::components::papyrus::property;

      using property_status     = dovah::loaded_forms::components::papyrus::property_status;
      using property_value_type = dovah::loaded_forms::components::papyrus::property_type;

      static constexpr const size_t ColumnName  = 0;
      static constexpr const size_t ColumnType  = 1;
      static constexpr const size_t ColumnValue = 2;

   public:
      struct object_property_value {
         dovah::form_stub* form = nullptr;
         uint16_t alias_id = -1; // "no alias"
      };

      using property_value = std::variant<
         std::monostate, // only for clearing an inherited property value REFR-side
         //
         object_property_value,
         QString,
         int32_t,
         float,
         bool,
         //
         std::vector<object_property_value>,
         std::vector<QString>,
         std::vector<int32_t>,
         std::vector<float>,
         std::vector<bool>
      >;

      // Represents a property set on the ScriptObject, OR a property defined in the Papyrus 
      // script even if not explicitly set on the ScriptObject.
      class Property {
         public:
            struct Typeinfo {
               property_value_type underlying_type;
               QString name; // scriptname, pulled from the compiled PEX, when `underlying_type` is Form or Form[]
            };
            struct Binding {
               property_status status;
               property_value  value;

               property_value_type typecode() const;
            };

         public:
            QString name;
            QString docstring;
            std::optional<Typeinfo> type; // type as dictated by the compiled script; absent if the PEX is not loadable
            struct {
               std::optional<Binding> parent; // base form, if the form we're currently editing is a REFR
               std::optional<Binding> target; // form we're currently editing. NOTE: should have a value if clearing an inherited property value REFR-side!
            } bindings;
            //
            QString value_string; // cached; computed from `bindings`

            void clearParentBinding();
            void clearTargetBinding();
            void setParentBinding(const vmad_property&);
            void setTargetBinding(const vmad_property&);
            void setBindings(const vmad_property& parent, const vmad_property& target);

            bool valueTypeIsOrContainsForm() const;
            bool isOrContainsForm(const dovah::form_stub&) const;
            void recacheValueString();
            bool onFormDeletionImminent(const dovah::form_stub&); // returns true if anything about this property has changed

            // Used when the user wants to set this property's value. Should change the "target" binding, forcing the 
            // status to `defined_locally` and setting the value.
            void changeValueTo(const property_value&);

            std::optional<property_status> getComputedStatus() const;
      };
      
   protected:
      dovah::form_stub* attached_to = nullptr;
      struct {
         vmad_script* parent = nullptr;
         vmad_script* target = nullptr;
      } vmad_scripts;
      QString scriptname;
      QVector<Property> properties;
      
   protected slots:
      void formDeletionImminent(const dovah::form_stub*, bool is_just_flagged);
      void formRenumbered(const dovah::form_stub*, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID);
      void formsRenumberedEnMasse();

   protected:
      void _loadPropertiesFromPex(QMap<QString, Property>& dst);
      void _importProperties(QMap<QString, Property>& src); // moves elements from `src`
      
   public:
      DKPapyrusScriptObjectModel(QObject* parent = nullptr);
      ~DKPapyrusScriptObjectModel() {
         this->clearTarget();
      }

      void setTarget(dovah::form_stub&, vmad_script& target);
      void setTarget(dovah::form_stub&, vmad_script& target, vmad_script& parent);
      void clearTarget();
      void syncToTarget();
      
      #pragma region QAbstractItemModel overrides
         QModelIndex index(int row, int column, const QModelIndex& parent) const override;
         QModelIndex parent(const QModelIndex& index) const;
         int rowCount(const QModelIndex& parent) const override;
         int columnCount(const QModelIndex& item) const override;
         Qt::ItemFlags flags(const QModelIndex& index) const override;
         QVariant data(const QModelIndex& index, int role) const override;
         QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
      #pragma endregion

      const Property* row(int rowIndex) const noexcept;

      void clearPropertyValue(int rowIndex); // use script default
      void revertPropertyValue(int rowIndex); // revert to inherited value
      void setPropertyValue(int rowIndex, const property_value&);
};