#pragma once
#include <QLabel>
#include <QWidget>

class DovahscriptAuthorWidget : public QWidget {
   Q_OBJECT;
   public:
      DovahscriptAuthorWidget(QWidget* parent = nullptr);

      struct Link {
         QString text;
         QString url;
      };

      inline QVector<Link> links() const noexcept;
      void addLink(const QString& text, const QString& url);
      void addLink(Link);
      void removeLink(const Link&);
      void clearLinks();

      inline QString name() const noexcept { return this->_fields.name; }
      void setName(const QString&) noexcept;

   protected:
      struct RenderedLink : Link {
         QLabel* widget = nullptr;
      };

      struct {
         QString name;
         QVector<RenderedLink> links;
      } _fields;
      struct {
         QLabel*  name     = nullptr;
         QWidget* linkWrap = nullptr;
      } _subwidgets;
};