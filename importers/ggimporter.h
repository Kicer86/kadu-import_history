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

#ifndef GGIMPORTER_H
#define GGIMPORTER_H

#include <QString>

#include "importer.h"
#include "../gg.h"
#include "plugins/gadu_protocol/gadu-protocol.h"

class MemFile;

/**
 * @defgroup ggimporter "GG importer"
 * @{
 */
class ImportFromGG:public Importer
{
    Q_OBJECT

    QString decode(const QByteArray& msg, const Contact user);
    MemFile *arch;
    UinType owner_uin;

    typedef QList<UinType> UinsList;

    struct header       gg_header;
    struct index        gg_index;
    struct block        gg_block;
    struct msg_header  *gg_msg_header;
    struct message      gg_message;
    struct rcv_msg      gg_rcv_msg;

    Chat chatFromUinsList(const UinsList &uinsList) const;
    
  public:
    ImportFromGG(const Account& acc, QString file, QObject *p);
    void run();
};

/** @} */

#endif
