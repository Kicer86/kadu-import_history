/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Michał Walenciak <Kicer86@gmail.com>

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

#include <stdio.h>
#include <sqlite3.h>
#include <QDir>
#include <QMessageBox>
#include <QTextCodec>

#include "chat/message/message.h"
#include "contacts/contact-manager.h"
#include "plugins/history/history.h"
#include "misc/path-conversion.h"
#include "protocols/services/chat-image-service.h"

#include "gg8importer.h"

ImportFromGG8::ImportFromGG8(const Account& acc, QString d, QObject* p): Importer(acc, p), dir(d)
{
  //sprawdź czy w podanym katalogu znajduje się wszystko co trzeba
  QFileInfo archiveFile(dir + "/Archive.db");
  QDir imagesDir(dir + "/ImgCache");

  if (archiveFile.exists()==false)
  {
    QMessageBox::critical(0, tr("Error"), tr("There is no Archive.db file in %1 directory.").arg(dir));
    cancelImport();
    return;
  }

  if ( (noImages=imagesDir.exists()==false) )
    QMessageBox::warning(0, tr("Warning"), tr("There is no ImgCache directory in %1 directory.").arg(dir));
  else   //upewniamy się że w katalogu ~/.kadu jest katalog `images`
  {
    QDir images;
    images.mkpath(ChatImageService::imagesPath());
  }
}


QString ImportFromGG8::decode(QString msg)
{
  if (noImages)
    return msg;         //nie ma katalogu z obrazkami - wyjdź

  int pos=0;
  while ( (pos=msg.indexOf("<", pos))>=0)
  {
    int end=msg.indexOf(">", pos);    //znajdź tag zamykający

    if (msg.mid(pos,5)=="<img ")  //obrazek?
    {
      int pathBegin=msg.indexOf("src=", pos);          //poczatek adresu obrazka
      int pathEnd=msg.indexOf("\"", pathBegin+5);      //pomiń początkowy '"'
      QString path=msg.mid(pathBegin+5, pathEnd-pathBegin-5);  //wyłuskaj scieżkę

      //kopiuj plik do katalogu kadu
      int fileName=path.indexOf("ImgCache");           //trzeba takie cyrki bo raz jest / a raz \ i QFileInfo sobie z tym nie radzi
      path.remove(0, fileName+9);
      QFile srcImage(dir+"/ImgCache/"+path);
      srcImage.copy(ChatImageService::imagesPath()+path);

      qDebug() << dir+"/ImgCache/"+path << "to" << ChatImageService::imagesPath()+path;

      //podmien scieżkę
      msg.replace(pathBegin+5, pathEnd-pathBegin-5, "kaduimg:///"+path);

      qDebug() << "kaduimg://"+path;
    }

    pos=end+1;
  }
  return msg;
}


void ImportFromGG8::run()
{
  sqlite3 *db;
  QByteArray archiveFile=dir.toUtf8()+"/Archive.db";
  int rc;

  //otwórz bazę
  rc=sqlite3_open_v2( archiveFile.data(), &db, SQLITE_OPEN_READONLY, 0);
  if (rc)
  {
    QMessageBox::critical(0, tr("Error"), tr("Could not open database.").arg(dir));
    return;
  }

  //odczytaj liczbę wpisów
  sqlite3_stmt *stmt_entries;
  char entries_sql[]="SELECT * FROM sqlite_sequence";
  rc=sqlite3_prepare_v2(db, entries_sql, -1, &stmt_entries, 0);
  if (rc != SQLITE_OK)
  {
    QMessageBox::critical(0, tr("Error"), tr("Error while executing %1.").arg(entries_sql));
    return;
  }
  sqlite3_step(stmt_entries);
  int entries=sqlite3_column_int(stmt_entries, 0);
  sqlite3_finalize(stmt_entries);
  emit boundaries(1, entries);               //ustaw progress bar (liczba wpisów)

  //odczytaj zbiór czatów
  sqlite3_stmt *stmt_chats;
  const char chats_sql[]="SELECT chat_id, interlocutor_id FROM 'chats';";
  rc=sqlite3_prepare_v2(db, chats_sql, -1, &stmt_chats, 0);
  if (rc != SQLITE_OK)
  {
    QMessageBox::critical(0, tr("Error"), tr("Error while executing %1.").arg(chats_sql));
    return;
  }

  //pobierz dany czat
  while (cancel==false && sqlite3_step(stmt_chats)!=SQLITE_DONE)
  {
    int chat_id=sqlite3_column_int(stmt_chats, 0);
    int interlocutor_id=sqlite3_column_int(stmt_chats, 1);

    //odczytaj uiny rozmówców
    sqlite3_stmt *stmt_uin;
    char uin_sql[128];
    sprintf(uin_sql, "SELECT identification FROM interlocutors WHERE interlocutor_id='%d';", interlocutor_id);
    rc=sqlite3_prepare_v2(db, uin_sql, -1, &stmt_uin, 0);
    if (rc != SQLITE_OK)
    {
      QMessageBox::critical(0, tr("Error"), tr("Error while executing %1.").arg(uin_sql));
      return;
    }
    sqlite3_step(stmt_uin);
    QString uinsRaw=reinterpret_cast<const char*>(sqlite3_column_text(stmt_uin, 0));
    sqlite3_finalize(stmt_uin);

    //przetwórz pobrane uiny
    UinsList uinsList;
    QStringList uins=uinsRaw.split("-");
    foreach(QString uin, uins)
    uinsList << uin.toInt();

    //pobierz czat
    sqlite3_stmt *stmt_conversation;
    char conversation_sql[128];
    sprintf(conversation_sql, "SELECT * FROM communication_items WHERE chat_id='%d';",chat_id);

    rc=sqlite3_prepare_v2(db, conversation_sql, -1, &stmt_conversation, 0);
    if (rc != SQLITE_OK)
    {
      QMessageBox::critical(0, tr("Error"), tr("Error while executing %1.").arg(conversation_sql));
      return;
    }

    //przetwarzaj wpisy danego czatu
    while (sqlite3_step(stmt_conversation)!=SQLITE_DONE)
    {
      //odczytaj różne dane
      bool sent=sqlite3_column_int(stmt_conversation, 2)==1;
      QDateTime date=QDateTime::fromString(reinterpret_cast<const char*>(sqlite3_column_text(stmt_conversation, 3)), "yyyy-MM-ddTHH:mm:ss");
      bool sms=sqlite3_column_int(stmt_conversation, 4)==1;
      QByteArray msg=reinterpret_cast<const char*>(sqlite3_column_text(stmt_conversation, 5));

      QTextCodec *codec = QTextCodec::codecForName("utf8");   //???? TODO: bez tego nie działa

      if (sms==false)
      {
        //dopisz wiadomość do historii
        Message message=Message::create();
        //gg8 nie przechowuje w historii informacji nt tego który z rozmówców z konferencji wysłał wiadomość (dziwne).
        //Użyjemy zatem pierwszego z listy.
        Contact user=sent? account.accountContact()
                     :ContactManager::instance()->byId(account, QString(uinsList[0]), ActionCreateAndAdd);
        message.setMessageChat(chatFromUinsList(uinsList));
        message.setMessageSender(user);
        message.setContent(decode(codec->toUnicode(msg)));
        message.setSendDate(date);
        message.setReceiveDate(date);
        message.setType(sent? MessageTypeSent : MessageTypeReceived);

        History::instance()->currentStorage()->appendMessage(message);
      }
      else //sesemes ;]
      {
        QString recipient=QString(uinsList[0]);
        History::instance()->currentStorage()->appendSms(recipient, codec->toUnicode(msg), date);
      }

      position++;                                               //dodano kolejny wpis -> postęp progress baru
    }
    sqlite3_finalize(stmt_conversation);
  }

  sqlite3_finalize(stmt_chats);

  sqlite3_close(db);
}
