/*
 Klasa-szkielet dla poszczególnych importerów
 Copyright (C) 2010  Michał Walenciak
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>. 
 */


#include <QMessageBox>

#include "chat/chat-manager.h"
#include "contacts/contact-manager.h"
#include "contacts/contact-set.h"
#include "importer.h"


Importer::Importer(const Account &acc, QObject* parent): 
    QThread(parent), cancel(false), account(acc)
{
  if (QMessageBox::No==QMessageBox::warning(0, tr("Warning"), tr("This is beta version of Gadu-Gadu archive import pluggin!\n"
      "Before you start, backup your kadu history (~/.kadu/history directory).\n\n"
      "It's highly recommended to switch kadu to offline status.\n"
      "Do not browse your history while import is in progress.\n"
      "Ready to continue?"),
      QMessageBox::Yes|QMessageBox::No,QMessageBox::No)
     ) 
  {
    cancelImport();
    return;
  }
}


void Importer::cancelImport()
{
  cancel=true;
}


bool Importer::canceled() const
{
  return cancel;
}


int Importer::getPosition() const
{
  return position;
}


Chat Importer::chatFromUinsList(const UinsList& uinsList) const
{
  ContactSet contacts;
  foreach(UinType uin, uinsList)
  {
    contacts.insert(ContactManager::instance()->byId(account, QString::number(uin), ActionCreateAndAdd));
  }
  return ChatManager::instance()->findChat(contacts);
}