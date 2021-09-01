#include "hyperlink_confirm.h"
#include <array>
#include <QBoxLayout>
#include <QClipboard>
#include <QDesktopServices>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QPushButton>
#include "../../../helpers/qt/url.h"

namespace {
   bool _has_user_info(const QUrl& url) {
      return !url.userInfo().isEmpty();
   }

   bool _is_website_link(const QUrl& url) {
      if (url.port(-1) != -1)
         return false;
      if (!cobb::qt::url_is_web_scheme(url))
         return false;
      if (_has_user_info(url))
         return false;
      return true;
   }
   bool _is_nexusmods_domain(const QUrl& url) {
      auto host = url.host();
      if (host.startsWith("www.", Qt::CaseInsensitive))
         host = host.remove(0, 4);
      if (host.compare("nexusmods.com", Qt::CaseInsensitive) != 0)
         return false;
      return true;
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
      auto path = cobb::qt::split_url_path(url); // "mods", mod ID, "files", file ID
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
      //
      // The caller should run the following first, on its own:
      //  - _is_website_link
      //  - _is_nexusmods_domain
      //
      auto path = cobb::qt::split_url_path(url);
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
         auto query = cobb::qt::split_url_query(url);
         for (auto& pair : query) {
            const auto& key = pair.first;
            if (key.compare("tab", Qt::CaseInsensitive) != 0)
               return false;
         }
      }
      return true;
   }
   bool _is_nexus_user_link(const QUrl& url) {
      //
      // The caller should run the following first, on its own:
      //  - _is_website_link
      //  - _is_nexusmods_domain
      //
      auto path = cobb::qt::split_url_path(url);
      int  id   = 1;
      if (path.size() < 2)
         return false;
      if (path[0].compare("users", Qt::CaseInsensitive) != 0) {
         if (path[1].compare("users", Qt::CaseInsensitive) != 0)
            return false;
         ++id;
         if (path.size() <= id)
            return false;
      }
      bool ok = false;
      path[id].toInt(&ok); // user ID
      if (!ok)
         return false;
      //
      // Verify query:
      //
      if (url.hasQuery()) {
         // don't allow arbitrary query strings e.g. "?tags_yes[]=1032"
         auto query = cobb::qt::split_url_query(url);
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
   if (_is_nexus_nxm_link(url)) {
      intro->setText(tr("This is a download link for a mod uploaded to NexusMods. Open it in your default web browser?"));
   } else {
      bool matched = false;
      if (_is_website_link(url)) {
         matched = true;
         if (_is_nexusmods_domain(url)) {
            if (_is_nexus_mod_link(url)) {
               intro->setText(tr("This link leads to a mod uploaded to NexusMods. Open it in your default web browser?"));
            } else if (_is_nexus_user_link(url)) {
               intro->setText(tr("This link leads to a user profile on NexusMods. Open it in your default web browser?"));
            } else {
               matched = false;
            }
         } else {
            matched = false;
         }
      }
      if (!matched) {
         intro->setText(tr("Open this link in your default web browser?"));
         outro->setText(tr("Avoid opening links that you don't recognize. You shouldn't go somewhere just because you were told to by a script that you got from some random person on the Internet."));
      }
   }
   outro->setVisible(!outro->text().isEmpty());
}