/*
   Klasa ułatwiająca pracę na zmapowanych plikach
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

#include <QIODevice>

class MemFile: public QIODevice
{
    QFile file;
    qint64 position;
    qint64 size;
    uchar *ptr;

  public:
    MemFile(QString fname);

    virtual bool open(OpenMode mode);
    virtual void close();
    virtual bool seek(qint64 pos);
    virtual qint64 pos() const;
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char*, qint64);
    
    qint64 read(char *data, qint64 maxlen);
    QByteArray read(qint64 maxlen);
};
