#include "DovahscriptAuthorWidget.h"
#include <QGridLayout>
#include <QUrl>
#include "../script_window/hyperlink_confirm.h"

DovahscriptAuthorWidget::DovahscriptAuthorWidget(QWidget* parent) : QWidget(parent) {
   auto* layout = new QGridLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);
   //
   {
      auto* widget = this->_subwidgets.name = new QLabel;
      widget->setWordWrap(true);
      widget->setTextFormat(Qt::PlainText);
      layout->addWidget(widget, 0, 0);
      //
      auto font = widget->font();
      font.setBold(true);
      widget->setFont(font);
   }
   //
   {
      auto* widget = this->_subwidgets.linkWrap = new QWidget;
      layout->addWidget(widget, 1, 0);
      widget->setLayout(new QBoxLayout(QBoxLayout::Direction::Down, widget));
      widget->layout()->setContentsMargins(0, 0, 0, 0);
   }
}

QVector<DovahscriptAuthorWidget::Link> DovahscriptAuthorWidget::links() const noexcept {
   QVector<Link> out;
   for (auto& l : this->_fields.links)
      out.push_back((Link)l);
   return out;
}
void DovahscriptAuthorWidget::addLink(const QString& text, const QString& url) {
   this->addLink({
      .text = text,
      .url  = url,
   });
}
void DovahscriptAuthorWidget::addLink(Link l) {
   RenderedLink out { l };
   out.widget = new QLabel;
   //
   out.widget->setWordWrap(true);
   out.widget->setTextFormat(Qt::RichText);
   out.widget->setText(
      QString("<a href=\"%2\">%1</a>")
         .arg(out.text.toHtmlEscaped())
         .arg(out.url.toHtmlEscaped())
   );
   out.widget->setOpenExternalLinks(false);
   QObject::connect(out.widget, &QLabel::linkActivated, this, [this](const QString& link) {
      auto* confirm = new ScriptWindowHyperlinkConfirmDialog(link, this);
      confirm->open();
   });
   this->_subwidgets.linkWrap->layout()->addWidget(out.widget);
   //
   this->_fields.links.push_back(std::move(out));
}
void DovahscriptAuthorWidget::removeLink(const Link& l) {
   auto& list = this->_fields.links;
   int   size = list.size();
   int   i;
   for (i = 0; i < size; ++i) {
      auto& e = list[i];
      if (e.text == l.text && e.url == l.url)
         break;
   }
   if (i >= size)
      return;
   auto& e = list[i];
   if (e.widget) {
      auto* layout = this->_subwidgets.linkWrap->layout();
      auto  i      = layout->indexOf(e.widget);
      if (i >= 0) {
         auto* item = layout->takeAt(i);
         delete item;
      }
      delete e.widget;
   }
   list.removeAt(i);
}
void DovahscriptAuthorWidget::clearLinks() {
   this->_fields.links.clear();
   //
   QLayout*     layout = this->_subwidgets.linkWrap->layout();
   QLayoutItem* child;
   while ((child = layout->takeAt(0)) != nullptr) {
      if (auto* widget = child->widget())
         delete widget;
      delete child;
   }
}

void DovahscriptAuthorWidget::setName(const QString& n) noexcept {
   this->_subwidgets.name->setText(n);
}