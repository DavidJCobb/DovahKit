#include "./DKScriptObjectDialog.h"
#include <array>
#include <cassert>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStandardItemModel>
#include "helpers/type_traits/is_std_vector.h"
#include "../widget-models/DKFormVMADModel.h"

namespace {
   constexpr const bool allow_incomplete_polishing = true;
}

DKScriptObjectDialog::DKScriptObjectDialog(QWidget& parent, QModelIndex scriptModelIndex) : QDialog(&parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonOK,     &QPushButton::clicked, this, &QDialog::accept);

   static_assert(allow_incomplete_polishing, "POLISH: Show scriptname and form it's attached to in the window title.");

   this->scriptQMI = scriptModelIndex;
   {
      auto* prop_view = this->ui.properties;
      prop_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      prop_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      if (scriptModelIndex.isValid()) {
         prop_view->setModel((QAbstractItemModel*)scriptModelIndex.model());
         prop_view->setRootIndex(scriptModelIndex);
         assert(dynamic_cast<DKFormVMADModel*>(scriptModelIndex.model()) != nullptr);
      }
      if (auto* header = prop_view->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* vh = prop_view->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
         vh->setVisible(false);
      }
      QObject::connect(prop_view->selectionModel(), &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
         this->_showSelectedProperty();
      });
   }

   {
      auto* widget = this->ui.arrayTable;
      widget->setCornerButtonEnabled(false);
      widget->setHorizontalHeaderLabels({ tr("#", "array-value table header: index"), tr("Value", "array-value table header: value")});
      if (auto* header = widget->horizontalHeader()) {
         header->setStretchLastSection(true);
      }
      if (auto* vh = widget->verticalHeader()) {
         vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
         vh->setVisible(false);
      }
      QObject::connect(widget, &QTableWidget::currentCellChanged, this, [this](int currentRow, int currentColumn, int previousRow, int previousColumn) {
         auto  prop_qmi   = this->_selectedPropertyQMI();
         auto* vmad_model = (DKFormVMADModel*)this->scriptQMI.model();
         if (!vmad_model || !prop_qmi.isValid()) {
            return;
         }

         auto prop_info = vmad_model->getPropertyWorkingMetadata(prop_qmi);
         if (!prop_info.typeinfo.underlying.has_value()) {
            return;
         }

         auto* widget = this->ui.arrayTable;

         switch (prop_info.typeinfo.underlying.value()) {
            using enum DKFormVMADModel::property_type;
            case array_of_boolean:
            case array_of_float32:
            case array_of_integer:
            case array_of_object:
            case array_of_string:
               break;
            default:
               return;
         }

         auto value = vmad_model->getPropertyWorkingValue(prop_qmi);

         const auto blockers = std::array{
            QSignalBlocker(this->ui.valueWidget_bool),
            QSignalBlocker(this->ui.valueWidget_float),
            QSignalBlocker(this->ui.valueWidget_int),
            QSignalBlocker(this->ui.valueWidget_string),

            QSignalBlocker(this->ui.valueWidget_form),
            QSignalBlocker(this->ui.valueWidget_ref),
         };

         switch (prop_info.typeinfo.underlying.value()) {
            using enum DKFormVMADModel::property_type;
            case array_of_boolean:
               this->ui.valueWidget_bool->setChecked(std::get<std::vector<bool>>(value)[currentRow]);
               break;
            case array_of_float32:
               this->ui.valueWidget_float->setValue(std::get<std::vector<float>>(value)[currentRow]);
               break;
            case array_of_integer:
               this->ui.valueWidget_int->setValue(std::get<std::vector<int32_t>>(value)[currentRow]);
               break;
            case array_of_object:
               if (prop_info.underlying_form_typeinfo.is_alias_type) {
                  auto& item = std::get<std::vector<DKFormVMADModel::object_property_value>>(value)[currentRow];

                  auto* quest    = item.form;
                  auto  alias_id = item.alias_id;

                  static_assert(false, "TODO: we don't have any UI for alias-type properties!!!!!");
               } else {
                  dovah::form_stub* stub = std::get<std::vector<DKFormVMADModel::object_property_value>>(value)[currentRow].form;
                  this->ui.valueWidget_form->setFormStub(stub);
                  this->ui.valueWidget_ref->setRef(stub);
               }
               break;
            case array_of_string:
               this->ui.valueWidget_string->setPlainText(std::get<std::vector<QString>>(value)[currentRow]);
               break;
         }
      });
   }

   #pragma region Value-change handlers
   QObject::connect(this->ui.valueWidget_bool, &QCheckBox::stateChanged, this, [this](int state) {
      bool checked = (state == Qt::CheckState::Checked);
      this->_setCurrentlyFocusedValue(QVariant::fromValue(checked));
   });
   QObject::connect(this->ui.valueWidget_float, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this](double value) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue<float>(value));
   });
   QObject::connect(this->ui.valueWidget_int, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int value) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(value));
   });
   QObject::connect(this->ui.valueWidget_string, &QPlainTextEdit::textChanged, this, [this](QString text) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(text));
   });
   //
   QObject::connect(this->ui.valueWidget_form, &FormPicker::formChanged, this, [this](dovah::form_stub* value) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(value));
   });
   QObject::connect(this->ui.valueWidget_ref, &DKObjectReferencePicker::refChanged, this, [this](dovah::form_stub* ref) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(ref));
   });
   static_assert(false, "TODO: we don't have any UI for alias-type properties!!!!!");
   #pragma endregion

   this->_showSelectedProperty();
}

QModelIndex DKScriptObjectDialog::_selectedPropertyQMI() const {
   if (!this->scriptQMI.isValid())
      return {};
   auto* sm = this->ui.properties->selectionModel();
   if (!sm)
      return {};
   auto rows = sm->selectedRows();
   if (rows.empty())
      return {};
   return rows[0];
}

void DKScriptObjectDialog::_setCurrentlyFocusedValue(QVariant v) {
   auto  prop_qmi   = this->_selectedPropertyQMI();
   auto* vmad_model = (DKFormVMADModel*)this->scriptQMI.model();
   if (!vmad_model || !prop_qmi.isValid()) {
      return;
   }

   auto prop_info = vmad_model->getPropertyWorkingMetadata(prop_qmi);
   if (!prop_info.typeinfo.underlying.has_value()) {
      return;
   }
   auto row = this->ui.arrayTable->currentRow();

   auto underlying = prop_info.typeinfo.underlying.value();
   switch (underlying) {
      using enum DKFormVMADModel::property_type;

      case boolean:
         vmad_model->setPropertyWorkingValue(prop_qmi, v.toBool());
         break;
      case array_of_boolean:
         {
            using value_type = std::vector<bool>;

            auto prior = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(prior));
            std::get<value_type>(prior)[row] = v.toBool();
         }
         break;

      case float32:
         vmad_model->setPropertyWorkingValue(prop_qmi, v.toFloat());
         break;
      case array_of_float32:
         {
            using value_type = std::vector<float>;

            auto prior = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(prior));
            std::get<value_type>(prior)[row] = v.toFloat();
         }
         break;

      case integer:
         vmad_model->setPropertyWorkingValue(prop_qmi, v.toInt());
         break;
      case array_of_integer:
         {
            using value_type = std::vector<int32_t>;

            auto prior = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(prior));
            std::get<value_type>(prior)[row] = v.toInt();
         }
         break;

      case string:
         vmad_model->setPropertyWorkingValue(prop_qmi, v.toString());
         break;
      case array_of_string:
         {
            using value_type = std::vector<QString>;

            auto prior = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(prior));
            std::get<value_type>(prior)[row] = v.toString();
         }
         break;

      case object:
         {
            DKFormVMADModel::object_property_value data;
            if (v.canConvert<dovah::form_stub*>()) {
               data.form = v.value<dovah::form_stub*>();
            } else {
               static_assert(false, "TODO: we need a way to handle aliases. ideally we should define a common type and QMetaType for them");
            }
            vmad_model->setPropertyWorkingValue(prop_qmi, data);
         }
         break;
      case array_of_object:
         {
            DKFormVMADModel::object_property_value data;
            if (v.canConvert<dovah::form_stub*>()) {
               data.form = v.value<dovah::form_stub*>();
            } else {
               static_assert(false, "TODO: we need a way to handle aliases. ideally we should define a common type and QMetaType for them");
            }

            {
               using value_type = std::vector<DKFormVMADModel::object_property_value>;

               auto prior = vmad_model->getPropertyWorkingValue(prop_qmi);
               assert(std::holds_alternative<value_type>(prior));
               std::get<value_type>(prior)[row] = data;
            }
         }
         break;
   }
}

void DKScriptObjectDialog::_showSelectedProperty() {
   auto  prop_qmi   = this->_selectedPropertyQMI();
   auto* vmad_model = (DKFormVMADModel*)this->scriptQMI.model();

   if (!vmad_model || !prop_qmi.isValid()) {
      this->ui.propertyType->setText(tr("Property type: <nothing selected>"));
      this->ui.arrayEditingLayout->setVisible(false);
      this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }

   auto prop_info = vmad_model->getPropertyWorkingMetadata(prop_qmi);

   {
      auto& disp_name = prop_info.typeinfo.display_typename;
      if (disp_name.isEmpty()) {
         this->ui.propertyType->setText(tr("Property type: <unknown>"));
      } else {
         this->ui.propertyType->setText(tr("Property type: %1").arg(disp_name));
      }
   }

   this->ui.valuePage_object->setEnabled(prop_info.has_underlying_form_type());
   this->ui.valuePage_ref->setEnabled(prop_info.has_underlying_form_type());

   if (!prop_info.status.has_value()) {
      this->ui.arrayEditingLayout->setVisible(false);
      this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }
   switch (prop_info.status.value()) {
      case DKFormVMADModel::property_status::defined_only_on_base:
         this->ui.arrayEditingLayout->setVisible(false);
         this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
         return;
   }

   auto value = vmad_model->getPropertyWorkingValue(prop_qmi);
   std::visit(
      [this, &prop_qmi, vmad_model, &prop_info](const auto& v) {
         using value_type = std::decay_t<decltype(v)>;

         auto _show_widgets = [this, &prop_info]<typename ValueType>() {
            if constexpr (std::is_same_v<ValueType, bool>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_bool);
            } else if constexpr (std::is_same_v<ValueType, float>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_float);
            } else if constexpr (std::is_same_v<ValueType, int>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_int);
            } else if constexpr (std::is_same_v<ValueType, QString>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_string);
            } else if constexpr (std::is_same_v<ValueType, DKFormVMADModel::object_property_value>) {
               if (dovah::form_type_info::form_type_is_reference(prop_info.underlying_form_type())) {
                  this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_ref);
               } else {
                  this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_object);
               }
            }
         };

         if constexpr (cobb::is_std_vector<value_type>) {
            using element_type = typename value_type::value_type;

            this->ui.arrayEditingLayout->setVisible(true);

            auto* table = this->ui.arrayTable;
            table->clearContents();
            table->setRowCount(v.size());

            for (size_t i = 0; i < v.size(); ++i) {
               {  // Show index
                  auto* item = new QTableWidgetItem;
                  table->setItem(i, 0, item);
                  item->setText(QString::number(i));
               }
               auto* item = new QTableWidgetItem;
               table->setItem(i, 1, item);

               QString text = vmad_model->getPropertyWorkingValueStringified(prop_qmi, i);
               item->setText(text);
               item->setToolTip(text);
            }
            
            _show_widgets.template operator()<element_type>();
         } else {
            this->ui.arrayEditingLayout->setVisible(false);
            this->ui.arrayTable->clearContents();

            _show_widgets.template operator()<value_type>();
         }
      },
      value
   );
}