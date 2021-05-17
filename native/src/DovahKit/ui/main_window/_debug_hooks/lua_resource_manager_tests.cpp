#include "lua_resource_manager_tests.h"
#include <array>
#include <vector>
#include <QBoxLayout>
#include <QColor>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QTimer>

namespace {
   using namespace DovahKitDebug;
   using referent_t      = DovahKitTESTQVariantWrappedSmartPointerReferent;
   using smart_pointer_t = DovahKitTESTQVariantWrappedSmartPointer<referent_t>;

   std::array colors = {
      QColor(255,   0, 0),
      QColor(255, 128, 0),
      QColor(255, 192, 0),
      QColor(255, 255, 0),
      QColor(192, 255, 0),
      QColor(128, 192, 0),
      QColor( 64, 160, 0),
      QColor( 32, 128, 128),
      QColor(  0,  64, 192),
      QColor(  0,   0, 255),
      QColor( 64,   0, 255),
      QColor(128,   0, 255),
      QColor(255,   0, 255),
      QColor(255,   0, 128),
   };

   static void _shift_colors() {
      std::rotate(colors.begin(), colors.begin() + 1, colors.end());
      DovahKitTESTQVariantWrappedSmartPointerReferentRegistry::get().updateAllReferents();
   }

   QModelIndex _get_combobox_item_qmi(QComboBox* widget, int index) {
      auto* model = widget->model();
      assert(model);
      return model->index(index, widget->modelColumn(), widget->rootModelIndex());
   }
}

namespace DovahKitDebug {
   /*static*/ DovahKitTESTQVariantWrappedSmartPointerReferent* DovahKitTESTQStyledItemDelegate::_extract_referent(const QModelIndex& index) {
      auto value = index.data(Qt::DecorationRole);
      if (value.isValid() && value.canConvert<QObject*>()) {
         if (auto* object = value.value<QObject*>())
            return qobject_cast<referent_t*>(object);
      }
      return nullptr;
   }

   void DovahKitTESTQStyledItemDelegate::initStyleOption(QStyleOptionViewItem* option, const QModelIndex& index) const {
      QStyledItemDelegate::initStyleOption(option, index);
      //
      if (auto* referent = _extract_referent(index)) {
         int i = referent->value % colors.size();
         //
         QPixmap pixmap(option->decorationSize);
         pixmap.fill(colors[i]);
         option->icon = QIcon(pixmap);
         option->features |= QStyleOptionViewItem::HasDecoration;
      }
   }

   extern void run_lua_resource_manager_tests(QWidget* parent) {
      qRegisterMetaType<smart_pointer_t>(); // ensure the metatype is registered at run-time
      //
      auto* dialog = new QDialog(parent);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
      dialog->setLayout(layout);
      //
      auto* select = new QComboBox(dialog);
      select->setItemDelegate(new DovahKitTESTQStyledItemDelegate(select));
      layout->addWidget(select);
      for (int i = 0; i < colors.size() / 2 + 1; ++i) {
         select->addItem(QString("Color %1").arg(i));
         auto qmi = _get_combobox_item_qmi(select, i);

         auto* referent = DovahKitTESTQVariantWrappedSmartPointerReferentRegistry::get().createReferent();
         auto  smart    = smart_pointer_t(referent, qmi);
         referent->value = i;
         //
         select->setItemData(i, QVariant::fromValue<smart_pointer_t>(smart), Qt::DecorationRole);
      }
      //
      {
         auto* button = new QPushButton("Cycle colors");
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, dialog, []() {
            _shift_colors();
         });
      }
      {
         auto* timer  = new QTimer(dialog);
         timer->setInterval(500);
         timer->setSingleShot(false);
         QObject::connect(timer, &QTimer::timeout, dialog, []() {
            _shift_colors();
         });
         //
         auto* start = new QPushButton("Start cycling");
         layout->addWidget(start);
         QObject::connect(start, &QPushButton::clicked, dialog, [timer]() {
            timer->start();
         });
         //
         auto* stop = new QPushButton("Stop cycling");
         layout->addWidget(stop);
         QObject::connect(stop, &QPushButton::clicked, dialog, [timer]() {
            timer->stop();
         });
      }
      //
      QObject::connect(dialog, &QDialog::finished, dialog, &QDialog::deleteLater);
      dialog->show();
   }
}