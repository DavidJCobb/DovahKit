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

#include "editor/form_stub_meta_type.h"
#include "ui/types/quest_alias.h"

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
         assert(dynamic_cast<const DKFormVMADModel*>(scriptModelIndex.model()) != nullptr);
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
         auto  prop_qmi = this->_selectedPropertyQMI();
         auto* vmad_model = (DKFormVMADModel*)this->scriptQMI.model();
         if (!vmad_model || !prop_qmi.isValid()) {
            return;
         }

         auto prop_info = vmad_model->getPropertyWorkingMetadata(prop_qmi);
         if (!prop_info.typeinfo.underlying.has_value()) {
            return;
         }

         auto* widget = this->ui.arrayTable;

         if (!prop_info.typeinfo.underlying.value().is_array)
            return;

         auto value = vmad_model->getPropertyWorkingValue(prop_qmi);

         const auto blockers = std::array{
            QSignalBlocker(this->ui.valueWidget_bool),
            QSignalBlocker(this->ui.valueWidget_float),
            QSignalBlocker(this->ui.valueWidget_int),
            QSignalBlocker(this->ui.valueWidget_string),

            QSignalBlocker(this->ui.valueWidget_form),
            QSignalBlocker(this->ui.valueWidget_ref),
         };

         switch (prop_info.typeinfo.underlying.value().base) {
            using enum ui::types::papyrus::single_value_type;
            case boolean:
               this->ui.valueWidget_bool->setChecked(std::get<std::vector<bool>>(value)[currentRow]);
               break;
            case float32:
               this->ui.valueWidget_float->setValue(std::get<std::vector<float>>(value)[currentRow]);
               break;
            case integer:
               this->ui.valueWidget_int->setValue(std::get<std::vector<int32_t>>(value)[currentRow]);
               break;
            case string:
               this->ui.valueWidget_string->setPlainText(std::get<std::vector<QString>>(value)[currentRow]);
               break;

            case alias:
               {
                  auto& item = std::get<std::vector<DKFormVMADModel::object_property_value>>(value)[currentRow];

                  auto* quest    = item.form;
                  auto  alias_id = item.alias_id;

                  this->ui.valueWidget_alias->setQuestAlias({
                     .quest    = quest,
                     .alias_id = alias_id,
                  });
               }
               break;

            case form:
               {
                  dovah::form_stub* stub = std::get<std::vector<DKFormVMADModel::object_property_value>>(value)[currentRow].form;
                  this->ui.valueWidget_form->setFormStub(stub);
                  this->ui.valueWidget_ref->setRef(stub);
               }
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
   QObject::connect(this->ui.valueWidget_string, &QPlainTextEdit::textChanged, this, [this]() {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(this->ui.valueWidget_string->toPlainText()));
   });
   //
   QObject::connect(this->ui.valueWidget_form, &FormPicker::formChanged, this, [this](dovah::form_stub* value) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(value));
   });
   QObject::connect(this->ui.valueWidget_ref, &DKObjectReferencePicker::refChanged, this, [this](dovah::form_stub* ref) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(ref));
   });
   QObject::connect(this->ui.valueWidget_alias, &DKQuestAliasPicker::aliasChanged, this, [this](const ui::types::quest_alias& alias) {
      this->_setCurrentlyFocusedValue(QVariant::fromValue(alias));
   });
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
   switch (underlying.base) {
      using enum ui::types::papyrus::single_value_type;

      case boolean:
         if (underlying.is_array) {
            vmad_model->setPropertyWorkingValue(prop_qmi, v.toBool());
         } else {
            using value_type = std::vector<bool>;

            auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(data));
            std::get<value_type>(data)[row] = v.toBool();
            vmad_model->setPropertyWorkingValue(prop_qmi, data);
         }
         break;

      case float32:
         if (underlying.is_array) {
            vmad_model->setPropertyWorkingValue(prop_qmi, v.toFloat());
         } else {
            using value_type = std::vector<float>;

            auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(data));
            std::get<value_type>(data)[row] = v.toFloat();
            vmad_model->setPropertyWorkingValue(prop_qmi, data);
         }
         break;

      case integer:
         if (underlying.is_array) {
            vmad_model->setPropertyWorkingValue(prop_qmi, v.toInt());
         } else {
            using value_type = std::vector<int32_t>;

            auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(data));
            std::get<value_type>(data)[row] = v.toInt();
            vmad_model->setPropertyWorkingValue(prop_qmi, data);
         }
         break;

      case string:
         if (underlying.is_array) {
            vmad_model->setPropertyWorkingValue(prop_qmi, v.toString());
         } else {
            using value_type = std::vector<QString>;

            auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
            assert(std::holds_alternative<value_type>(data));
            std::get<value_type>(data)[row] = v.toString();
            vmad_model->setPropertyWorkingValue(prop_qmi, data);
         }
         break;

      case alias:
         {
            auto d = v.value<ui::types::quest_alias>();

            DKFormVMADModel::object_property_value val;
            val.form     = d.quest;
            val.alias_id = d.alias_id;

            if (underlying.is_array) {
               vmad_model->setPropertyWorkingValue(prop_qmi, val);
            } else {
               using value_type = std::vector<DKFormVMADModel::object_property_value>;

               auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
               assert(std::holds_alternative<value_type>(data));
               std::get<value_type>(data)[row] = val;
               vmad_model->setPropertyWorkingValue(prop_qmi, data);
            }
         }
         break;

      case form:
         {
            DKFormVMADModel::object_property_value val;
            val.form = v.value<dovah::form_stub*>();

            if (underlying.is_array) {
               vmad_model->setPropertyWorkingValue(prop_qmi, val);
            } else {
               using value_type = std::vector<DKFormVMADModel::object_property_value>;

               auto data = vmad_model->getPropertyWorkingValue(prop_qmi);
               assert(std::holds_alternative<value_type>(data));
               std::get<value_type>(data)[row] = val;
               vmad_model->setPropertyWorkingValue(prop_qmi, data);
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