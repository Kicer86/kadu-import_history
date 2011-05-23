/*
 K lasa ułatwiająca pracę na zmapowanych plikach*
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

#include "memfile.h"


MemFile::MemFile(QString fname): QIODevice(), position(0)
{
  file.setFileName(fname);
}


bool MemFile::open(QIODevice::OpenMode mode)
{
  bool st= file.open(mode) && (ptr=file.map(0, file.size()));
  if (!st)
    file.close();
  else
    size=file.size();
  return st;
}

void MemFile::close()
{
  file.unmap(ptr);
  file.close();
  QIODevice::close();
}


bool MemFile::seek(qint64 pos)
{
  return (position=pos)<size;
}


qint64 MemFile::pos() const
{
  return position;
}


qint64 MemFile::readData(char *data, qint64 maxlen)
{
  if (position < size && file.isOpen())
  {
    qint64 readed=0;
    while (readed<maxlen && position<size)
    {
      data[readed++]=ptr[position++];
    }
    return readed;
  }
  else
    return -1;
}


qint64 MemFile::writeData(const char*, qint64)
{
  return -1;
}


qint64 MemFile::read(char* data, qint64 maxlen)
{
  return readData(data, maxlen);
}


QByteArray MemFile::read(qint64 maxlen)
{
  QByteArray data;
  if (position < size && file.isOpen())
  {
    qint64 readed=0;
    while (readed<maxlen && position<size)
    {
      readed++;
      data.append(ptr[position++]);
    }
  }

  return data;
}
