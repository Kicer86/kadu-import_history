/*
  obiekt wtyczki
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
#include "core/core.h"
#include "debug.h"
#include "gui/actions/action-description.h"
#include "gui/windows/kadu-window.h"
#include "misc/misc.h"

#include "import_history.h"
#include "gui/gui.h"


/** @ingroup import_history
 * @{
 */



ImportHistory::ImportHistory(): QObject()
{
}


ImportHistory::~ImportHistory()
{
}


int ImportHistory::init(bool)
{
  kdebugf();
  //dopisz się do menu kadu
  importMenuActionDescription = new ActionDescription(
    this, ActionDescription::TypeMainMenu, "importHistoryAction",
    this, SLOT(importHistory(QAction *, bool)),KaduIcon(),tr("Import history")
  );
  Core::instance()->kaduWindow()->insertMenuActionDescription(importMenuActionDescription, KaduWindow::MenuTools, 5);
  kdebugf2();
  return 0;
}


void ImportHistory::done()
{
  kdebugf();
  Import::destroyInstance();
  Core::instance()->kaduWindow()->removeMenuActionDescription(importMenuActionDescription);
  kdebugf2();
}


void ImportHistory::importHistory(QAction *, bool)
{
  Import::show();
}

Q_EXPORT_PLUGIN2(import_history, ImportHistory)

/** @} */
