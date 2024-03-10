#include "./DKScriptObjectDialog.h"
#include <array>
#include <cassert>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>
#include "helpers/type_traits/is_std_vector.h"
#include "../widget-models/DKBoundScriptModel.h"
#include "../widget-models/DKBoundScriptListModel.h"

#include "editor/form_stub_meta_type.h"
#include "ui/types/quest_alias.h"

namespace {
   constexpr const bool allow_incomplete_polishing = true;
}

namespace vmad {
   using namespace dovah::loaded_forms::components::papyrus;
}

DKScriptObjectDialog::DKScriptObjectDialog(QWidget& parent, QModelIndex scriptModelIndex) : QDialog(&parent) {
   this->ui.setupUi(this);

   this->setWindowFlag(Qt::WindowContextHelpButtonHint);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      if (this->script_model) {
         delete this->script_model;
         this->script_model = nullptr;
      }
      this->reject();
   });
   QObject::connect(this->ui.buttonOK, &QPushButton::clicked, this, [this]() {
      ((DKBoundScriptListModel*)this->script_qmi.model())->replaceScriptModelFor(this->script_qmi, this->script_model);
      this->accept();
   });

   static_assert(allow_incomplete_polishing, "POLISH: Show scriptname and form it's attached to in the window title.");

   this->script_qmi   = scriptModelIndex;
   this->script_model = ((DKBoundScriptListModel*)scriptModelIndex.model())->createScriptModelFor(scriptModelIndex);

   this->ui.valueWidget_float->setMinimum(std::numeric_limits<float>::lowest());
   this->ui.valueWidget_float->setMaximum(std::numeric_limits<float>::max());

   {
      auto* prop_view = this->ui.properties;
      prop_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      prop_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      if (scriptModelIndex.isValid()) {
         prop_view->setModel(this->script_model);
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
      prop_view->setIconSize({ 16, 16 });
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
         auto qmi = this->_selectedPropertyQMI();
         if (!this->script_model || !qmi.isValid()) {
            return;
         }

         const auto info_opt = this->script_model->infoForProperty(qmi);
         if (!info_opt.has_value())
            return;
         const auto& info = info_opt.value();

         if (!info.is_array)
            return;

         const auto value_opt = this->script_model->getPropertyValueElement(qmi, currentRow);
         if (!value_opt.has_value())
            return;
         const auto& value = value_opt.value();

         auto* widget = this->ui.arrayTable;

         const auto blockers = std::array{
            QSignalBlocker(this->ui.valueWidget_bool),
            QSignalBlocker(this->ui.valueWidget_float),
            QSignalBlocker(this->ui.valueWidget_int),
            QSignalBlocker(this->ui.valueWidget_string),

            QSignalBlocker(this->ui.valueWidget_form),
            QSignalBlocker(this->ui.valueWidget_ref),
         };

         switch (vmad::scalar_property_type_for(info.raw_type)) {
            using enum vmad::property_type;
            case boolean:
               this->ui.valueWidget_bool->setChecked(std::get<bool>(value));
               break;
            case float32:
               this->ui.valueWidget_float->setValue(std::get<float>(value));
               break;
            case integer:
               this->ui.valueWidget_int->setValue(std::get<int32_t>(value));
               break;
            case string:
               this->ui.valueWidget_string->setPlainText(std::get<QString>(value));
               break;
            case object:
               if (info.is_alias()) {
                  auto& item = std::get<ui::types::quest_alias>(value);
                  this->ui.valueWidget_alias->setQuestAlias({
                     .quest    = item.quest,
                     .alias_id = item.alias_id,
                  });
               } else if (info.is_form()) {
                  dovah::form_stub* stub = std::get<dovah::form_stub*>(value);
                  this->ui.valueWidget_form->setFormStub(stub);
                  this->ui.valueWidget_ref->setRef(stub);
               }
               break;
         }
      });
   }

   QObject::connect(this->ui.buttonAutoFillAll, &QPushButton::clicked, this, [this]() {
      if (!this->script_model)
         return;
      this->script_model->autoFillAllProperties();
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueAutoFill, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;
      this->script_model->autoFillProperty(qmi);
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueClearOrMakeLocal, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      const auto info_opt = this->script_model->infoForProperty(qmi);
      if (!info_opt.has_value())
         return;
      const auto& info = info_opt.value();

      if (info.is_defined()) {
         this->script_model->clearProperty(qmi);
      } else {
         this->script_model->makePropertyLocal(qmi);
      }
      this->_showSelectedProperty();
   });
   QObject::connect(this->ui.buttonValueRevert, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;
      this->script_model->revertProperty(qmi);
      this->_showSelectedProperty();
   });

   QObject::connect(this->ui.arrayButtonAdd, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [&changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               v.emplace_back();
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });
   QObject::connect(this->ui.arrayButtonRemove, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      size_t row;
      if (auto opt = this->_currentArrayElementIndex(); opt.has_value()) {
         row = opt.value();
      } else {
         return;
      }

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [row, &changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (row >= v.size())
                  return;
               v.erase(v.begin() + row);
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });
   QObject::connect(this->ui.arrayButtonMoveUp, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      size_t row;
      if (auto opt = this->_currentArrayElementIndex(); opt.has_value()) {
         row = opt.value();
      } else {
         return;
      }

      if (row == 0) // can't move
         return;

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [row, &changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (v.size() <= row)
                  return;
               if constexpr (std::is_same_v<value_type, std::vector<bool>>) { // fucking vector-of-bool...
                  bool a = v[row];
                  bool b = v[row - 1];
                  v[row]     = b;
                  v[row - 1] = a;
               } else {
                  std::swap(v[row], v[row - 1]);
               }
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });
   QObject::connect(this->ui.arrayButtonMoveDown, &QPushButton::clicked, this, [this]() {
      auto qmi = this->_selectedPropertyQMI();
      if (!this->script_model || !qmi.isValid())
         return;

      size_t row;
      if (auto opt = this->_currentArrayElementIndex(); opt.has_value()) {
         row = opt.value();
      } else {
         return;
      }

      auto value_opt = this->script_model->getPropertyValue(qmi);
      if (!value_opt.has_value())
         return;
      auto& value   = value_opt.value();
      bool  changed = false;
      std::visit(
         [row, &changed](auto& v) {
            using value_type = std::decay_t<decltype(v)>;
            if constexpr (cobb::is_std_vector<value_type>) {
               if (row + 1 >= v.size())
                  return;
               if constexpr (std::is_same_v<value_type, std::vector<bool>>) { // fucking vector-of-bool...
                  bool a = v[row];
                  bool b = v[row + 1];
                  v[row]     = b;
                  v[row + 1] = a;
               } else {
                  std::swap(v[row], v[row + 1]);
               }
               changed = true;
            }
         },
         value
      );
      if (changed)
         this->script_model->setPropertyLocalValue(qmi, value);
   });

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
   QObject::connect(this->ui.valueWidget_form, &DKFormPicker::formChanged, this, [this](dovah::form_stub* value) {
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

   if (this->script_model->anyPropertiesDiscardedOnLoad()) {
      QMessageBox::warning(
         this,
         tr("Some properties have been removed"),
         tr("Some Papyrus property values were discarded, either because the properties in question don't actually exist on the script (as defined in the relevant PEX files) or because the values were invalid.")
      );
      this->script_model->forgetAnyPropertiesWereDiscardedOnLoad(); // so we're only warned once, if we commit our changes
   }
}

std::optional<size_t> DKScriptObjectDialog::_currentArrayElementIndex() const {
   auto* sm = this->ui.arrayTable->selectionModel();
   if (!sm)
      return {};
   auto rows = sm->selectedRows();
   if (rows.empty())
      return {};
   auto row = rows[0].row();
   if (row < 0)
      return {};
   return row;
}
QModelIndex DKScriptObjectDialog::_selectedPropertyQMI() const {
   if (!this->script_qmi.isValid())
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
   auto qmi = this->_selectedPropertyQMI();
   if (!this->script_model || !qmi.isValid()) {
      return;
   }

   const auto info_opt = this->script_model->infoForProperty(qmi);
   if (!info_opt.has_value())
      return;
   const auto& info = info_opt.value();
   const auto  row  = this->ui.arrayTable->currentRow();

   switch (vmad::scalar_property_type_for(info.raw_type)) {
      using enum vmad::property_type;

      case boolean:
         if (!info.is_array) {
            this->script_model->setPropertyLocalValue(qmi, v.toBool());
         } else {
            this->script_model->setPropertyLocalValueElement(qmi, v.toBool(), row);
         }
         break;

      case float32:
         if (!info.is_array) {
            this->script_model->setPropertyLocalValue(qmi, v.toFloat());
         } else {
            this->script_model->setPropertyLocalValueElement(qmi, v.toFloat(), row);
         }
         break;

      case integer:
         if (!info.is_array) {
            this->script_model->setPropertyLocalValue(qmi, v.toInt());
         } else {
            this->script_model->setPropertyLocalValueElement(qmi, v.toInt(), row);
         }
         break;

      case string:
         if (!info.is_array) {
            this->script_model->setPropertyLocalValue(qmi, v.toString());
         } else {
            this->script_model->setPropertyLocalValueElement(qmi, v.toString(), row);
         }
         break;

      case object:
         if (info.is_alias()) {
            ui::types::quest_alias val = v.value<ui::types::quest_alias>();

            if (!info.is_array) {
               this->script_model->setPropertyLocalValue(qmi, val);
            } else {
               this->script_model->setPropertyLocalValueElement(qmi, val, row);
            }
         } else if (info.is_form()) {
            dovah::form_stub* val = v.value<dovah::form_stub*>();

            if (!info.is_array) {
               this->script_model->setPropertyLocalValue(qmi, val);
            } else {
               this->script_model->setPropertyLocalValueElement(qmi, val, row);
            }
         }
         break;
   }
}

void DKScriptObjectDialog::_showSelectedProperty() {
   auto qmi = this->_selectedPropertyQMI();
   if (!this->script_model || !qmi.isValid()) {
      this->ui.propertyType->setText(tr("Property type: <nothing selected>"));
      this->ui.arrayEditingLayout->setVisible(false);
      this->ui.arrayEditingLayout->setEnabled(false);
      this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }
   const auto info_opt = this->script_model->infoForProperty(qmi);
   if (!info_opt.has_value()) {
      this->ui.propertyType->setText(tr("Property type: <nothing selected>"));
      this->ui.arrayEditingLayout->setVisible(false);
      this->ui.arrayEditingLayout->setEnabled(false);
      this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }
   const auto& info = info_opt.value();

   auto type_name = this->script_model->data(qmi.siblingAtColumn(DKBoundScriptModel::Column::Type), Qt::DisplayRole).toString();
   if (type_name.isEmpty()) {
      this->ui.propertyType->setText(tr("Property type: <unknown>"));
   } else {
      this->ui.propertyType->setText(tr("Property type: %1").arg(type_name));
   }

   this->ui.valuePage_object->setEnabled(info.is_form());
   this->ui.valuePage_ref->setEnabled(info.is_form());

   if (info.inherited) {
      this->ui.buttonValueRevert->setEnabled(info.defined_locally);
   } else {
      this->ui.buttonValueRevert->setEnabled(false);
   }

   if (info.is_defined()) {
      this->ui.buttonValueClearOrMakeLocal->setText(tr("Clear Value"));
   } else {
      this->ui.buttonValueClearOrMakeLocal->setText(tr("Edit Value"));
   }

   if (!info.is_defined() || !info.defined_locally) {
      this->ui.arrayEditingLayout->setVisible(false);
      this->ui.arrayEditingLayout->setEnabled(false);
      this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_defaulted);
      return;
   }

   this->ui.arrayEditingLayout->setVisible(info.is_array);
   this->ui.arrayEditingLayout->setEnabled(info.is_array);

   auto value_opt = this->script_model->getPropertyValue(qmi);
   if (!value_opt.has_value())
      return;
   std::visit(
      [this, &info, &qmi](const auto& v) {
         using value_type = std::decay_t<decltype(v)>;

         auto _show_widgets = [this, &info, &v]<typename ValueType>() {
            if constexpr (std::is_same_v<ValueType, bool>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_bool);
            } else if constexpr (std::is_same_v<ValueType, float>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_float);
            } else if constexpr (std::is_same_v<ValueType, int>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_int);
            } else if constexpr (std::is_same_v<ValueType, QString>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_string);
            } else if constexpr (std::is_same_v<ValueType, ui::types::quest_alias>) {
               this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_alias);
            } else if constexpr (std::is_same_v<ValueType, dovah::form_stub*>) {
               dovah::form_type type = info.native_type.value_or(dovah::form_type::none);
               if (dovah::form_type_is_reference(type)) {
                  this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_ref);
               } else {
                  this->ui.singleValueWrap->setCurrentWidget(this->ui.valuePage_object);
               }
            }

            this->ui.buttonValueAutoFill->setEnabled(std::is_same_v<ValueType, dovah::form_stub*>);
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

               QString text = ui::bound_script_models::stringify(v[i]);
               item->setText(text);
               item->setToolTip(text);
            }
            
            _show_widgets.template operator()<element_type>();
         } else {
            this->ui.arrayEditingLayout->setVisible(false);
            this->ui.arrayTable->clearContents();

            _show_widgets.template operator()<value_type>();
            
            const auto blockers = std::array{
               QSignalBlocker(this->ui.valueWidget_bool),
               QSignalBlocker(this->ui.valueWidget_float),
               QSignalBlocker(this->ui.valueWidget_int),
               QSignalBlocker(this->ui.valueWidget_string),

               QSignalBlocker(this->ui.valueWidget_form),
               QSignalBlocker(this->ui.valueWidget_ref),
            };

            if constexpr (std::is_same_v<value_type, bool>) {
               this->ui.valueWidget_bool->setChecked(v);
            } else if constexpr (std::is_same_v<value_type, float>) {
               this->ui.valueWidget_float->setValue(v);
            } else if constexpr (std::is_same_v<value_type, int>) {
               this->ui.valueWidget_int->setValue(v);
            } else if constexpr (std::is_same_v<value_type, QString>) {
               this->ui.valueWidget_string->setPlainText(v);
            } else if constexpr (std::is_same_v<value_type, ui::types::quest_alias>) {
               this->ui.valueWidget_alias->setRequiredScriptname(info.scriptname);
               this->ui.valueWidget_alias->setQuestAlias(v);
            } else if constexpr (std::is_same_v<value_type, dovah::form_stub*>) {
               if (dovah::form_type_is_reference(info.native_type.value_or(dovah::form_type::none))) {
                  this->ui.valueWidget_ref->setRequiredScriptname(info.scriptname);
                  this->ui.valueWidget_ref->setRef(v);
               } else {
                  if (info.native_type.has_value())
                     this->ui.valueWidget_form->setAllowedFormType(info.native_type.value());
                  else
                     this->ui.valueWidget_form->allowAllFormTypes();

                  this->ui.valueWidget_form->setRequiredScriptname(info.scriptname);
                  this->ui.valueWidget_form->setFormStub(v);
               }
            }
         }
      },
      value_opt.value()
   );
}