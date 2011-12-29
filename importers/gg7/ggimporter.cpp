/*
  Wątek importujący archiwum Gadu-Gadu
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

#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QTimer>
#include <QTextCodec>

#include <libgadu.h>

#include "accounts/account-manager.h"
#include "message/message.h"
#include "configuration/configuration-file.h"
#include "contacts/contact-set.h"
#include "contacts/contact-manager.h"
#include "misc/misc.h"
#include "plugins/gadu_protocol/helpers/gadu-formatter.h"
#include "plugins/history/history.h"

#include "ggimporter.h"
#include "crc32.h"
#include "misc/memfile.h"

/** @ingroup ggimporter
 * @{
 */

ImportFromGG::ImportFromGG(const Account &acc, QString file, QObject *p): Importer(acc, p)
{
  //sprawdź czy plik istnieje
  QFile efile(file);
  if (!efile.exists())
  {
    QMessageBox::critical(0, tr("Error"), tr("File does no exist."));
    cancelImport();
    return;
  }

  if (account.isNull() || !History::instance()->currentStorage())
  {
    QMessageBox::critical(0, tr("Error"), tr("Could not find any Gadu-Gadu account."));
    cancelImport();
    return;
  }

  //wstępnie przeanalizuj plik z historią
  arch=new MemFile(file);                         //plik z archiwum - do odczytu danych
  arch->open(QIODevice::ReadOnly);
  /* odczytaj nagłówek pliku */
  arch->read(reinterpret_cast<char*>(&gg_header),sizeof(gg_header));

  owner_uin=gg_header.uin ^ 0xFFFFFD66;

  /* sprawdź czy uin sie zgadza */
  unsigned int acc_uin=account.id().toInt();
  if (acc_uin!=owner_uin)
  {
    if (QMessageBox::Yes==QMessageBox::warning(0, tr("Warning"),
        tr("It seems that it is not your Gadu-Gadu archive.\n"
           "The uin saved in archives.dat file, and your current uin are different.\n"
           "Cancel import?"),
        QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)
       )
      cancelImport();
  }
}


/* troche brzydkiego kodu ;) częsciowo skopiowanego z modułu history-migration*/
void ImportFromGG::run()
{
  //kodowanie historii
  QTextCodec::setCodecForCStrings(QTextCodec::codecForName("cp1250"));

  if (cancel)
    return;

  /* przesuń się na index */
  arch->seek(gg_header.index_off);

  /* oblicz ilość sekcji w indeksie */
  int index_entries=gg_header.index_size/sizeof(gg_index);

  /* odczytaj kolejne sekcje */
  for (int i=0;i<index_entries;i++)
  {
    arch->read(reinterpret_cast<char*>(&gg_index), sizeof(gg_index));
    if (i!=1) 
      continue;         //pierwsza sekcja (0) zawiera smieci, druga(1) wiadomosci, inne nas nie interesują
    /* zapamiętaj pozycje w pliku, bo zaraz bedziemy po nim jeździć */
    qint64 pos=arch->pos();

    /* offset bloku danych liczony względem sekcji danych (data_off w strukturze nagłówka) */
    /* przesuń się na pierwszy blok */
    arch->seek(gg_header.data_off + gg_index.first_block_off);
    emit boundaries(gg_index.first_block_off,gg_index.last_block_off);     //ustaw progress bar

    /* analizuj kolejne bloki */
    do
    {
      arch->read(reinterpret_cast<char*>(&gg_block), sizeof(gg_block));          //wczytaj nagłówek bloku

      /* odczytaj wszystkie nagłówki wiadomości znajdujące się za nagłówkiem */
      int msg_entries=gg_block.block_size/sizeof(struct msg_header);  //liczba wiadomości
      gg_msg_header=new struct msg_header[msg_entries];

      arch->read(reinterpret_cast<char*>(gg_msg_header), sizeof(struct msg_header) *msg_entries);
      for (int j=0;j<msg_entries;j++)
      {
        arch->seek(gg_header.data_off + gg_msg_header[j].msg_off);
        /* odczytaj nagłówek wiadomości */
        arch->read(reinterpret_cast<char*>(&gg_message), sizeof(gg_message));

        Message message=Message::create();

        /* na podstawie message.recivers rozsądź czy wiadomość przychodząca czy wychodząca */
        /* jeśli message.recivers==0 to wiadomość przychodząca */
        /* jeśli message.recivers>0 i uin inny niż w nagłówku to przychodząca konferencja */
        /* jeśli message.recivers>0 i uin jak w nagłówku to wychodząca (może również to być konferencja) */
        if (gg_message.recivers==0)   //przychodząca
        {
          /* doczytaj ciąg dalszy wiadomości */
          arch->read(reinterpret_cast<char*>(&gg_rcv_msg), sizeof(gg_rcv_msg));
          /* odczytaj treść wiadomości */
          QByteArray msg=arch->read(gg_rcv_msg.len);

          int uin = gg_message.sender_uin;
          UinsList uinsList;
          uinsList << uin;
          Chat chat=chatFromUinsList(uinsList);

          Contact user=ContactManager::instance()->byId(account, QString::number(uin), ActionCreateAndAdd);
          message.setMessageChat(chat);
          message.setMessageSender(user);
          message.setContent(decode(msg, user));
          message.setSendDate(QDateTime::fromTime_t (gg_message.send_time));
          message.setReceiveDate(QDateTime::fromTime_t (gg_rcv_msg.rcv_time));
          message.setType(MessageTypeReceived);

          History::instance()->currentStorage()->appendMessage(message);
          //1.  argument (osoba/by z którymi gadamy) 2. ten kto pisał czyli w tym wypadku dwa razy to samo
          //History::instance()->currentStorage()->appendMessage(uins, gg_message.sender_uin, decode ( msg,uins[0] ),false,gg_message.send_time,true,gg_rcv_msg.rcv_time );
        }
        else                    //wychodząca (konferencja) lub przychodząca konferencja
        {
          /* w pliku kolejno:
              0x0C            [n] * 4 bajty  numerki odbiorców
              0x0C + [n] * 4  4 bajty  prawdopodobnie czas dostarczenia wiadomości do odbiorcy
              0x10 + [n] * 4  4 bajty  długość wiadomości (len)
              0x14 + [n] * 4  [len] bajtów  wiadomość
          */
          quint32* us= new quint32[gg_message.recivers];
          quint32 time;
          quint32 len;

          arch->read(reinterpret_cast<char*>(us), sizeof(quint32) *gg_message.recivers);
          arch->read(reinterpret_cast<char*>(&time), sizeof(quint32));
          arch->read(reinterpret_cast<char*>(&len), sizeof(quint32));

          /* odczytaj treść wiadomości */
          QByteArray msg=arch->read(len);
          UinsList uins;
          bool outgoing= owner_uin==gg_message.sender_uin;
          if (outgoing==false)                  //konferencja lub wiadomość przychodząca
            uins << gg_message.sender_uin;      //dopisz tego co przysyła też do listy odbiorców

          for (int k=0; k<gg_message.recivers; k++)
            if (us[k]!=owner_uin)
              uins << us[k];    //stare gg (chyba <6.0) traktuje zwykle rozmowy jak konferencje dodając osobe wysylającą (własciciela) do listy odbiorców, wyrzuć go

          Contact user=outgoing? account.accountContact()
                                 :ContactManager::instance()->byId(account, QString::number(gg_message.sender_uin), ActionCreateAndAdd);
          message.setMessageChat(chatFromUinsList(uins));
          message.setMessageSender(user);
          message.setContent(decode(msg, user));
//           message.setContent(decode(msg, uins[0]));   //1. uin ? nie pamięta skąd to sie wzięło, widocznie tak jest ;)

          message.setSendDate(QDateTime::fromTime_t (gg_message.send_time));
          message.setReceiveDate(QDateTime::fromTime_t (gg_rcv_msg.rcv_time));
          message.setType(outgoing ? MessageTypeSent : MessageTypeReceived);

          History::instance()->currentStorage()->appendMessage(message);
        }
      }
      /* przesuń się do kolejnego bloku */
      delete [] gg_msg_header;
      if (gg_block.next_block!=0)
        arch->seek(gg_header.data_off + gg_block.next_block);
      position=gg_block.next_block;
    }
    while (gg_block.next_block!=0 && !cancel);   //pracuj dopóki cancel==false
    /* powrót do sekcji */
    arch->seek(pos);
  }

  arch->close();
  return;
}


/* rozszyfruj wiadomość i zwróć ją w sformatowanej postaci*/
/* po tekscie może pojawić się rich text */
/* http://toxygen.net/libgadu/protocol/#ch1.6 */
QString ImportFromGG::decode(const QByteArray& msg, const Contact user)      //uin potrzebny w razie pojawienia sie obrazka
{
  QString decoded_msg;
  QByteArray format;
  bool formatting=false;
  unsigned char key=0xff;
  int j=0, position=0, header=3;  //header - do pomijania nagłówka rich textu (0x02 + 2 bajty na długosc richa)

  while (j<msg.size())
  {
    quint8 c=msg.at(j);
    quint8 decoded_char=c^key;         //deszyfracja ;)
    if (formatting)
    {
      if (header>0)
      {
        format.append(decoded_char);
        position++;
      }
      else
        header--;
    }
    else
      if (!decoded_char)   //koniec, jeśli cos zostało jeszcze (i<msg.size()) to jest to formatowanie
        formatting=true;
      else
        decoded_msg.append(decoded_char);
    key=c;
    j++;
  }

  //sprawdź formatowanie w poszukiwaniu obrazków
  QByteArray nformat;
  position=0;

  unsigned char *array= (reinterpret_cast<unsigned char*>(format.data()));

  while (position<format.size())
  {
    struct gg_msg_richtext_format frm;
    struct gg_msg_richtext_image img;

    memcpy(&frm, array + position,sizeof(frm));
    for (unsigned int i=0;i<sizeof(frm);i++)
      nformat.append(format[position++]);

    if (frm.font & 0x08)   //kolor
      for (unsigned int i=0;i<sizeof(struct gg_msg_richtext_color);i++)
        nformat.append(format[position++]);

    if (frm.font & 0x80)   //obrazek
    {
      memcpy(&img, array + position,sizeof(img));
      for (unsigned int i=0;i<sizeof(img);i++)
        nformat.append(format[position++]);

      //sprawdź poprawność nagłówka - najwyraźniej kiedys był inny i powoduje to problemy
      //ewentualnie fake ;)
      if (img.unknown1!=0x109 || (img.size==20 && img.crc32==4567))
      {
        nformat="";                //calkowicie odrzuć formatowanie
        break;                     //zakoncz analize formatu
      }

      //wytnij dodatkowe informacje
      QByteArray file_name;
      position+=3;     //3 nieznane bajty
      while (array[position])      //nazwa
        file_name.append(array[position++]);
      position++;    //null
      position+=8;   //8 bajtów na crc i rozmiar (juz jest w nagłówku (img) )

      QFile img_file(profilePath(QString("images/%1-%2-%3-%4").arg(user.id().toInt()).arg(img.size).arg(img.crc32).arg(file_name.data())));
      img_file.open(QIODevice::WriteOnly);
      img_file.write(reinterpret_cast<char*>(array+position),img.size);
      img_file.close();
      position+=img.size;
    }
  }

  FormattedMessage fmt_msg=GaduFormatter::createMessage(account, user, decoded_msg, (unsigned char*) nformat.data(), nformat.size(), true);
  return fmt_msg.toHtml();
}


/** @} */
