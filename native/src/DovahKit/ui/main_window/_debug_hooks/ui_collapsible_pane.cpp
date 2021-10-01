#include "ui_collapsible_pane.h"
#include <QAction>
#include <QBoxLayout>
#include <QDialog>
#include <QGridLayout>
#include <QMessageBox>
#include <QPushButton>
#include "../../../widgets/DKCollapsiblePane.h"

#include <QPaintEvent>
#include <QPainter>

namespace DovahKitDebug {
   TestBadgeWidget::TestBadgeWidget(QWidget* parent) : QWidget(parent) {
      auto* l = this->_text = new QLabel(this);
      l->setAlignment(Qt::AlignCenter);
      {
         auto* layout = new QHBoxLayout(this);
         this->setLayout(layout);
         layout->setContentsMargins({ 4, 0, 4, 0 });
         layout->addWidget(l);
         layout->setSizeConstraint(QLayout::SetFixedSize);
      }
      {
         auto f = l->font();
         f.setBold(true);
         l->setFont(f);
         //
         auto p = l->palette();
         p.setColor(QPalette::ColorRole::WindowText, QColor(255, 255, 255));
         p.setColor(QPalette::ColorRole::Text, QColor(255, 255, 255));
         l->setPalette(p);
      }
      this->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
      //
      auto fm = QFontMetrics(l->font());
      this->setMinimumHeight(fm.height() * 1.2);
      this->setMinimumWidth(fm.height() * 1.2);
   }
   void TestBadgeWidget::setText(const QString& t) {
      this->_text->setText(t);
   }
   QSize TestBadgeWidget::minimumSizeHint() const noexcept {
      return this->_text->sizeHint();
   }
   QSize TestBadgeWidget::sizeHint() const noexcept {
      return this->_text->sizeHint();
   }
   void TestBadgeWidget::paintEvent(QPaintEvent* event) {
      auto  rect  = this->rect();
      qreal round = rect.height() / 2;
      //
      QPainter painter(this);
      painter.setPen(QPen(QBrush(), 0));
      painter.setBrush(QColor(255, 0, 0));
      painter.drawRoundedRect(rect, round, round);
   }

   TitleWithBadge::TitleWithBadge(QWidget* parent) : QWidget(parent) {
      this->_text  = new QLabel(this);
      this->_badge = new TestBadgeWidget(this);
      //
      auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
      this->setLayout(layout);
      layout->setSizeConstraint(QLayout::SetMinimumSize);
      layout->addWidget(this->_text);
      layout->addWidget(this->_badge);
      layout->setStretch(0, 0);
      layout->setStretch(1, 0);
      layout->addStretch(1);
      layout->setContentsMargins({ 0, 0, 0, 0 });
      //
      {
         auto f = this->_text->font();
         f.setBold(true);
         this->_text->setFont(f);
      }
   }

   void TitleWithBadge::setBadgeText(const QString& t) {
      this->_badge->setText(t);
   }
   void TitleWithBadge::setText(const QString& t) {
      this->_text->setText(t);
   }
}

namespace DovahKitDebug::features {
   /*static*/ void ui_collapsible_pane::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      dialog->setLayout(layout);
      //
      auto* pane = new DKCollapsiblePane(dialog);
      pane->setTitle("Test panel");
      layout->addWidget(pane);
      //
      if (auto* view = pane->viewport()) {
         auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, view);
         view->setLayout(layout);
         layout->addWidget(new QPushButton("Filler 1"));
         layout->addWidget(new QPushButton("Filler 2"));
         layout->addWidget(new QPushButton("Filler 3"));
         layout->addWidget(new QPushButton("Filler 4"));
      }
      //
      {
         auto* button = new QPushButton("Add Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [dialog, pane]() {
            auto  count  = pane->actions().count();
            auto* action = new QAction(QString("Action #%1").arg(count));
            action->setProperty("number", count);
            pane->addAction(action);
            //
            QObject::connect(action, &QAction::triggered, [dialog, count]() {
               QMessageBox::information(dialog, QString("Test"), QString("Triggered action %1.").arg(count));
            });
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove First Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto* target = list.first();
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove Middle Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            int   i      = list.size() / 2;
            auto* target = list[i];
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Remove Last Action");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto* target = list.back();
            if (target) {
               pane->removeAction(target);
               delete target;
            }
         });
         layout->addWidget(button);
      }
      {
         auto* button = new QPushButton("Scramble Actions");
         QObject::connect(button, &QPushButton::clicked, dialog, [pane]() {
            auto list = pane->actions();
            if (list.isEmpty())
               return;
            auto size = list.size();
            std::srand(std::time(0));
            int offset = std::rand() % size;
            int shift  = std::rand() % (size / 2);
            for (int i = 0; i < size; ++i) {
               int from = (offset + i) % size;
               int to   = (from + shift) % size;
               pane->insertAction(list[to], list[from]);
            }
         });
         layout->addWidget(button);
      }
      {
         auto* pane = new DKCollapsiblePane(dialog);
         pane->setTitle("Badge test panel");
         layout->addWidget(pane);
         //
         auto* badge = new TitleWithBadge(pane);
         badge->setBadgeText("1234");
         pane->setTitleWidget(badge);
      }
      //
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->show();
   }
}
