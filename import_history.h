/*
 Obiekt wtyczki
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

#ifndef ImportHistory_H
#define ImportHistory_H

#include <QtCore/QObject>
#include <plugins/generic-plugin.h>

class QAction;

class Import;
class ActionDescription;

/**
 * @defgroup import_history "Import history"
 * @{
 */
class ImportHistory : public QObject, public GenericPlugin
{
    Q_OBJECT
    Q_INTERFACES(GenericPlugin)

  public:
    ImportHistory();
    virtual ~ImportHistory();
    
    virtual int init(bool);
    virtual void done();

  private:
    ActionDescription *importMenuActionDescription;     //wpis do menu kadu
    Import* ImportDialog;                               //dialog

  private slots:
    void importHistory(QAction *, bool);
};

/** @} */

#endif

