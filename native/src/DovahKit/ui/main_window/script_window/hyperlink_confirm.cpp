#include "hyperlink_confirm.h"
#include <array>
#include <QBoxLayout>
#include <QClipboard>
#include <QDesktopServices>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>

namespace {
   bool _is_web_scheme(const QUrl& url) {
      auto scheme = url.scheme();
      if (scheme.compare("http", Qt::CaseInsensitive) == 0)
         return true;
      if (scheme.compare("https", Qt::CaseInsensitive) == 0)
         return true;
      return false;
   }
   bool _has_user_info(const QUrl& url) {
      return !url.userInfo().isEmpty();
   }

   QStringList _split_path(const QUrl& url) {
      QString     path = url.path(QUrl::NormalizePathSegments | QUrl::StripTrailingSlash);
      QStringList list;
      int size = path.size();
      int from = 0;
      for (int i = 0; i < size; ++i) {
         QChar c = path[i];
         if (c == '/' || c == '\\') {
            if (i != 0)
               list.push_back(QStringRef(&path, from, i - from).toString());
            from = i + 1;
            continue;
         }
      }
      if (from < size)
         list.push_back(QStringRef(&path, from, size - from).toString());
      return list;
   }
   std::vector<std::pair<QString, QString>> _split_query(const QUrl& url) {
      std::vector<std::pair<QString, QString>> out;
      QString query = url.query();
      int     size  = query.size();
      bool    key   = true;
      std::pair<QString, QString> current;
      //
      for (int i = 0; i <= size; ++i) {
         QChar c = i < size ? query[i] : QChar('&');
         if (key) {
            if (c == '&') {
               if (!current.first.isEmpty()) {
                  out.push_back(current);
                  current.first.clear();
                  current.second.clear();
               }
               continue;
            }
            if (c == '=') {
               key = false;
               continue;
            }
            current.first += c;
         } else {
            if (c == '&') {
               out.push_back(current);
               current.first.clear();
               current.second.clear();
               key = true;
               continue;
            }
            current.second += c;
         }
      }
      return out;
   }

   bool _is_nexus_nxm_link(const QUrl& url) {
      if (url.port(-1) != -1)
         return false;
      if (url.scheme().compare("nxm", Qt::CaseInsensitive) != 0)
         return false;
      if (_has_user_info(url))
         return false;
      if (url.hasQuery()) // don't allow query strings (change this if we learn of any valid ones)
         return false;
      //
      // nxm://game_name/mods/116/files/1658
      //
      // - host: game_name
      // - path: /mods/116/files/1658
      //
      {
         auto host = url.host(); // game name
         if (host.contains('.'))
            return false;
      }
      auto path = _split_path(url);
      if (path.size() < 4)
         return false;
      if (path[0].compare("mods", Qt::CaseInsensitive) != 0)
         return false;
      if (path[2].compare("files", Qt::CaseInsensitive) != 0)
         return false;
      bool ok = false;
      path[1].toInt(&ok);
      if (!ok)
         return false;
      path[3].toInt(&ok);
      if (!ok)
         return false;
      return true;
   }
   bool _is_nexus_mod_link(const QUrl& url) {
      if (url.port(-1) != -1)
         return false;
      if (!_is_web_scheme(url))
         return false;
      if (_has_user_info(url))
         return false;
      {
         auto host = url.host();
         if (host.startsWith("www.", Qt::CaseInsensitive))
            host = host.remove(0, 4);
         if (host.compare("nexusmods.com", Qt::CaseInsensitive) != 0)
            return false;
      }
      auto path = _split_path(url);
      if (path.size() < 3)
         return false;
      if (path[1].compare("mods", Qt::CaseInsensitive) != 0)
         return false;
      bool ok = false;
      path[2].toInt(&ok); // mod ID
      if (!ok)
         return false;
      //
      // Verify query:
      //
      if (url.hasQuery()) {
         // don't allow arbitrary query strings e.g. "?tags_yes[]=1032"
         auto query = _split_query(url);
         for (auto& pair : query) {
            const auto& key = pair.first;
            if (key.compare("tab", Qt::CaseInsensitive) != 0)
               return false;
         }
      }
      return true;
   }
}

ScriptWindowHyperlinkConfirmDialog::ScriptWindowHyperlinkConfirmDialog(const QString& url, QWidget* parent) : QDialog(parent), _targetUrl(url) {
   auto* layout = new QGridLayout(this);
   this->setLayout(layout);
   //
   auto* intro = new QLabel(this);
   auto* link  = new QLabel(url, this);
   auto* outro = new QLabel(this);
   layout->addWidget(intro, 0, 0);
   {
      auto* middle =  new QWidget(this);
      auto* midspan = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
      middle->setLayout(midspan);
      midspan->addStretch(1);
      midspan->addWidget(link, 5);
      midspan->addStretch(1);
      layout->addWidget(middle);
      middle->setMinimumWidth(320);
   }
   layout->addWidget(outro, 2, 0);
   intro->setWordWrap(true);
   outro->setWordWrap(true);
   outro->setAlignment(Qt::AlignJustify | Qt::AlignVCenter);
   link->setWordWrap(true);
   //
   auto* bar = new QWidget(this);
   auto* box = new QBoxLayout(QBoxLayout::Direction::LeftToRight, this);
   bar->setLayout(box);
   auto* buttonYes  = new QPushButton(tr("Yes, open the link"));
   auto* buttonCopy = new QPushButton(tr("Copy URL"));
   auto* buttonNo   = new QPushButton(tr("Cancel"));
   box->addStretch(1);
   box->addWidget(buttonYes);
   box->addWidget(buttonCopy);
   box->addWidget(buttonNo);
   box->addStretch(1);
   box->setMargin(0);
   layout->addWidget(bar, 3, 0);
   //
   QObject::connect(buttonYes, &QPushButton::clicked, this, [this]() {
      QDesktopServices::openUrl(this->_targetUrl);
      this->accept();
   });
   QObject::connect(buttonNo, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(buttonCopy, &QPushButton::clicked, this, [url]() {
      QGuiApplication::clipboard()->setText(url);
   });
   //
   if (_is_nexus_mod_link(url)) {
      intro->setText(tr("This link leads to a mod uploaded to NexusMods. Open it in your default web browser?"));
   } else if (_is_nexus_nxm_link(url)) {
      intro->setText(tr("This is a download link for a mod uploaded to NexusMods. Open it in your default web browser?"));
   } else {
      intro->setText(tr("Open this link in your default web browser?"));
      outro->setText(tr("Avoid opening links that you don't recognize. You shouldn't go somewhere just because you were told to by a script that you got from some random person on the Internet."));
   }
   outro->setVisible(!outro->text().isEmpty());
}